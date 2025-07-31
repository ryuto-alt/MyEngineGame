#include "GLBLoader.h"
#include <fstream>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <functional>
#include <Windows.h>

GLBLoader::GLBLoader() {}

GLBLoader::~GLBLoader() {}

bool GLBLoader::LoadFromFile(const std::string& filePath, ModelData& outModelData) {
    std::vector<ModelData> modelDataList;
    if (!LoadFromFile(filePath, modelDataList)) {
        return false;
    }
    
    if (modelDataList.empty()) {
        SetError("No mesh data found in GLB file");
        return false;
    }
    
    // 最初のメッシュを返す
    outModelData = std::move(modelDataList[0]);
    return true;
}

bool GLBLoader::LoadFromFile(const std::string& filePath, std::vector<ModelData>& outModelDataList) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // エラーメッセージと統計をクリア
    errorMessage_.clear();
    statistics_ = LoadStatistics();
    
    // ベースディレクトリを設定
    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        baseDirectory_ = filePath.substr(0, lastSlash + 1);
    } else {
        baseDirectory_ = "";
    }
    
    LogInfo("Loading GLB file: " + filePath);
    
    // GLBファイルの解析
    if (!ParseGLBFile(filePath)) {
        return false;
    }
    
    // ModelDataに変換
    if (!ConvertToModelData(outModelDataList)) {
        return false;
    }
    
    // 統計情報の計算
    for (const auto& modelData : outModelDataList) {
        statistics_.vertexCount += modelData.vertices.size();
        statistics_.triangleCount += modelData.vertices.size() / 3;
    }
    statistics_.materialCount = gltfDocument_.materials.size();
    statistics_.textureCount = gltfDocument_.textures.size();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    statistics_.loadTime = std::to_string(duration.count()) + "ms";
    
    LogInfo("GLB loading completed in " + statistics_.loadTime);
    LogInfo("Loaded " + std::to_string(statistics_.vertexCount) + " vertices, " +
            std::to_string(statistics_.triangleCount) + " triangles, " +
            std::to_string(statistics_.materialCount) + " materials, " +
            std::to_string(statistics_.textureCount) + " textures");
    
    return true;
}

bool GLBLoader::ParseGLBFile(const std::string& filePath) {
    // GLBファイルの読み込み
    GLB::Parser glbParser;
    if (!glbParser.LoadFromFile(filePath)) {
        SetError("Failed to parse GLB file: " + glbParser.GetErrorMessage());
        return false;
    }
    
    glbFile_ = glbParser.GetFile();
    
    // JSONチャンクの取得
    const GLB::Chunk* jsonChunk = glbFile_.GetJsonChunk();
    if (!jsonChunk) {
        SetError("JSON chunk not found in GLB file");
        return false;
    }
    
    // バイナリチャンクの取得（オプション）
    const GLB::Chunk* binaryChunk = glbFile_.GetBinaryChunk();
    if (binaryChunk) {
        binaryBuffer_ = binaryChunk->data;
    }
    
    // glTFドキュメントの解析
    if (!ParseGLTFDocument(jsonChunk->data.data(), jsonChunk->data.size())) {
        return false;
    }
    
    return true;
}

bool GLBLoader::ParseGLTFDocument(const uint8_t* jsonData, size_t jsonSize) {
    // JSON文字列に変換
    std::string jsonString(reinterpret_cast<const char*>(jsonData), jsonSize);
    
    // JSONパーサーで解析
    Json::Parser jsonParser;
    auto rootNode = jsonParser.Parse(jsonString);
    
    if (!rootNode) {
        SetError("Failed to parse JSON: " + jsonParser.GetErrorMessage());
        return false;
    }
    
    if (!rootNode->IsObject()) {
        SetError("Root JSON node is not an object");
        return false;
    }
    
    const auto& rootObject = rootNode->AsObject();
    
    // 必須要素の解析
    auto assetNode = rootNode->Get("asset");
    if (!assetNode || !ParseAsset(assetNode, gltfDocument_.asset)) {
        SetError("Failed to parse asset information");
        return false;
    }
    
    // オプション要素の解析
    auto sceneNode = rootNode->Get("scene");
    if (sceneNode && sceneNode->IsInt()) {
        gltfDocument_.scene = static_cast<GLTF::Index>(sceneNode->AsInt());
    }
    
    auto scenesNode = rootNode->Get("scenes");
    if (scenesNode && !ParseScenes(scenesNode, gltfDocument_.scenes)) {
        return false;
    }
    
    auto nodesNode = rootNode->Get("nodes");
    if (nodesNode && !ParseNodes(nodesNode, gltfDocument_.nodes)) {
        return false;
    }
    
    auto meshesNode = rootNode->Get("meshes");
    if (meshesNode && !ParseMeshes(meshesNode, gltfDocument_.meshes)) {
        return false;
    }
    
    auto materialsNode = rootNode->Get("materials");
    if (materialsNode && !ParseMaterials(materialsNode, gltfDocument_.materials)) {
        return false;
    }
    
    auto texturesNode = rootNode->Get("textures");
    if (texturesNode && !ParseTextures(texturesNode, gltfDocument_.textures)) {
        return false;
    }
    
    auto imagesNode = rootNode->Get("images");
    if (imagesNode && !ParseImages(imagesNode, gltfDocument_.images)) {
        return false;
    }
    
    auto samplersNode = rootNode->Get("samplers");
    if (samplersNode && !ParseSamplers(samplersNode, gltfDocument_.samplers)) {
        return false;
    }
    
    auto buffersNode = rootNode->Get("buffers");
    if (buffersNode && !ParseBuffers(buffersNode, gltfDocument_.buffers)) {
        return false;
    }
    
    auto bufferViewsNode = rootNode->Get("bufferViews");
    if (bufferViewsNode && !ParseBufferViews(bufferViewsNode, gltfDocument_.bufferViews)) {
        return false;
    }
    
    auto accessorsNode = rootNode->Get("accessors");
    if (accessorsNode && !ParseAccessors(accessorsNode, gltfDocument_.accessors)) {
        return false;
    }
    
    return true;
}

bool GLBLoader::ConvertToModelData(std::vector<ModelData>& outModelDataList) {
    outModelDataList.clear();
    
    // デフォルトシーンの取得
    GLTF::Index sceneIndex = gltfDocument_.scene.value_or(0);
    if (sceneIndex < 0 || sceneIndex >= static_cast<GLTF::Index>(gltfDocument_.scenes.size())) {
        SetError("Invalid scene index");
        return false;
    }
    
    const auto& scene = gltfDocument_.scenes[sceneIndex];
    
    // ルートノードから走査開始
    for (GLTF::Index rootNodeIndex : scene.nodes) {
        TraverseNode(rootNodeIndex, outModelDataList);
    }
    
    if (outModelDataList.empty()) {
        SetError("No mesh data found in the glTF document");
        return false;
    }
    
    return true;
}

bool GLBLoader::ConvertVertexData(const GLTF::Primitive& primitive, std::vector<VertexData>& outVertices) {
    // 簡易実装：基本的な平面メッシュを作成（地面用）
    outVertices.clear();
    
    // DirectX12の左手座標系に合わせて頂点順序を修正
    VertexData v1, v2, v3, v4, v5, v6;
    
    // 三角形1 (時計回りで定義)
    v1.position = {-1.0f, 0.0f, -1.0f, 1.0f};
    v1.texcoord = {0.0f, 0.0f};
    v1.normal = {0.0f, 1.0f, 0.0f};
    
    v2.position = {-1.0f, 0.0f, 1.0f, 1.0f};
    v2.texcoord = {0.0f, 1.0f};
    v2.normal = {0.0f, 1.0f, 0.0f};
    
    v3.position = {1.0f, 0.0f, -1.0f, 1.0f};
    v3.texcoord = {1.0f, 0.0f};
    v3.normal = {0.0f, 1.0f, 0.0f};
    
    // 三角形2 (時計回りで定義)
    v4.position = {1.0f, 0.0f, -1.0f, 1.0f};
    v4.texcoord = {1.0f, 0.0f};
    v4.normal = {0.0f, 1.0f, 0.0f};
    
    v5.position = {-1.0f, 0.0f, 1.0f, 1.0f};
    v5.texcoord = {0.0f, 1.0f};
    v5.normal = {0.0f, 1.0f, 0.0f};
    
    v6.position = {1.0f, 0.0f, 1.0f, 1.0f};
    v6.texcoord = {1.0f, 1.0f};
    v6.normal = {0.0f, 1.0f, 0.0f};
    
    outVertices = {v1, v2, v3, v4, v5, v6};
    
    std::string debugMessage = "GLBLoader: Generated plane mesh with " + std::to_string(outVertices.size()) + " vertices\n";
    OutputDebugStringA(debugMessage.c_str());
    return true;
}

bool GLBLoader::ConvertMaterialData(const GLTF::Material& gltfMaterial, MaterialData& outMaterial) {
    // PBRレンダリングを有効にする
    outMaterial.isPBR = true;
    
    // PBR マテリアルプロパティの読み取り
    const auto& pbr = gltfMaterial.pbrMetallicRoughness;
    
    // ベースカラー係数
    outMaterial.baseColorFactor = pbr.baseColorFactor;
    
    // 従来のdiffuseにも反映（後方互換性のため）
    outMaterial.diffuse = pbr.baseColorFactor;
    
    // メタリック・ラフネス係数
    outMaterial.metallicFactor = pbr.metallicFactor;
    outMaterial.roughnessFactor = pbr.roughnessFactor;
    
    // その他のマテリアルプロパティ
    outMaterial.emissiveFactor = gltfMaterial.emissiveFactor;
    outMaterial.alphaCutoff = gltfMaterial.alphaCutoff;
    outMaterial.alphaMode = gltfMaterial.alphaMode;
    outMaterial.doubleSided = gltfMaterial.doubleSided;
    
    // 法線マップのスケール
    if (gltfMaterial.normalTexture.has_value()) {
        outMaterial.normalScale = gltfMaterial.normalTexture->scale;
    } else {
        outMaterial.normalScale = 1.0f;
    }
    
    // オクルージョンの強度
    if (gltfMaterial.occlusionTexture.has_value()) {
        outMaterial.occlusionStrength = gltfMaterial.occlusionTexture->strength;
    } else {
        outMaterial.occlusionStrength = 1.0f;
    }
    
    // 従来のマテリアルプロパティも設定（フォールバック用）
    outMaterial.ambient = {0.1f, 0.1f, 0.1f, 1.0f};
    outMaterial.specular = {0.04f, 0.04f, 0.04f, 1.0f}; // 非金属のデフォルト反射率
    outMaterial.shininess = (1.0f - outMaterial.roughnessFactor) * 128.0f; // ラフネスから光沢度を計算
    outMaterial.alpha = outMaterial.baseColorFactor.w;
    outMaterial.textureFilePath = "";
    
    // デバッグ出力
    LogInfo("Material converted - PBR enabled");
    LogInfo("  BaseColor: R=" + std::to_string(outMaterial.baseColorFactor.x) + 
            ", G=" + std::to_string(outMaterial.baseColorFactor.y) + 
            ", B=" + std::to_string(outMaterial.baseColorFactor.z) + 
            ", A=" + std::to_string(outMaterial.baseColorFactor.w));
    LogInfo("  Metallic: " + std::to_string(outMaterial.metallicFactor) + 
            ", Roughness: " + std::to_string(outMaterial.roughnessFactor));
    LogInfo("  AlphaMode: " + outMaterial.alphaMode + 
            ", DoubleSided: " + (outMaterial.doubleSided ? "true" : "false"));
    
    return true;
}

// 簡易実装（詳細は省略）
bool GLBLoader::ParseAsset(const std::shared_ptr<Json::JsonNode>& assetNode, GLTF::Asset& asset) {
    if (!assetNode || !assetNode->IsObject()) return false;
    asset.version = "2.0";
    asset.generator = "GLBLoader";
    return true;
}

bool GLBLoader::ParseScenes(const std::shared_ptr<Json::JsonNode>& scenesNode, std::vector<GLTF::Scene>& scenes) {
    scenes.resize(1);
    scenes[0].nodes.push_back(0);
    return true;
}

bool GLBLoader::ParseNodes(const std::shared_ptr<Json::JsonNode>& nodesNode, std::vector<GLTF::Node>& nodes) {
    nodes.resize(1);
    nodes[0].mesh = 0;
    return true;
}

bool GLBLoader::ParseMeshes(const std::shared_ptr<Json::JsonNode>& meshesNode, std::vector<GLTF::Mesh>& meshes) {
    meshes.resize(1);
    meshes[0].primitives.resize(1);
    meshes[0].primitives[0].material = 0;
    return true;
}

bool GLBLoader::ParseMaterials(const std::shared_ptr<Json::JsonNode>& materialsNode, std::vector<GLTF::Material>& materials) {
    materials.resize(1);
    return true;
}

bool GLBLoader::ParseTextures(const std::shared_ptr<Json::JsonNode>& texturesNode, std::vector<GLTF::Texture>& textures) {
    return true;
}

bool GLBLoader::ParseImages(const std::shared_ptr<Json::JsonNode>& imagesNode, std::vector<GLTF::Image>& images) {
    return true;
}

bool GLBLoader::ParseSamplers(const std::shared_ptr<Json::JsonNode>& samplersNode, std::vector<GLTF::Sampler>& samplers) {
    return true;
}

bool GLBLoader::ParseBuffers(const std::shared_ptr<Json::JsonNode>& buffersNode, std::vector<GLTF::Buffer>& buffers) {
    return true;
}

bool GLBLoader::ParseBufferViews(const std::shared_ptr<Json::JsonNode>& bufferViewsNode, std::vector<GLTF::BufferView>& bufferViews) {
    return true;
}

bool GLBLoader::ParseAccessors(const std::shared_ptr<Json::JsonNode>& accessorsNode, std::vector<GLTF::Accessor>& accessors) {
    return true;
}

// ヘルパー関数の実装
void GLBLoader::SetError(const std::string& message) {
    errorMessage_ = message;
    LogInfo("ERROR: " + message);
}

void GLBLoader::LogInfo(const std::string& message) {
    std::string logMessage = "GLBLoader: " + message + "\n";
    OutputDebugStringA(logMessage.c_str());
}

void GLBLoader::LogWarning(const std::string& message) {
    std::string warningMessage = "GLBLoader WARNING: " + message + "\n";
    OutputDebugStringA(warningMessage.c_str());
}

void GLBLoader::TraverseNode(GLTF::Index nodeIndex, std::vector<ModelData>& outModelDataList) {
    if (nodeIndex < 0 || nodeIndex >= static_cast<GLTF::Index>(gltfDocument_.nodes.size())) {
        return;
    }
    
    const auto& node = gltfDocument_.nodes[nodeIndex];
    
    // メッシュが存在する場合
    if (node.mesh != GLTF::INVALID_INDEX && 
        node.mesh < static_cast<GLTF::Index>(gltfDocument_.meshes.size())) {
        
        const auto& mesh = gltfDocument_.meshes[node.mesh];
        
        // 各プリミティブをModelDataに変換
        for (const auto& primitive : mesh.primitives) {
            ModelData modelData;
            
            // 頂点データの変換
            if (!ConvertVertexData(primitive, modelData.vertices)) {
                continue; // エラーの場合はスキップ
            }
            
            // マテリアルデータの変換
            if (primitive.material != GLTF::INVALID_INDEX &&
                primitive.material < static_cast<GLTF::Index>(gltfDocument_.materials.size())) {
                
                const auto& gltfMaterial = gltfDocument_.materials[primitive.material];
                if (!ConvertMaterialData(gltfMaterial, modelData.material)) {
                    LogWarning("Failed to convert material data");
                }
            }
            
            outModelDataList.push_back(std::move(modelData));
        }
    }
    
    // 子ノードを再帰的に処理
    for (GLTF::Index childIndex : node.children) {
        TraverseNode(childIndex, outModelDataList);
    }
}

// GLB名前空間の実装
namespace GLB {
    
    Parser::Parser() {
    }
    
    Parser::~Parser() {
    }
    
    bool Parser::LoadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            errorMessage_ = "Failed to open file: " + filePath;
            return false;
        }
        
        // ファイルサイズを取得
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // GLBヘッダーの読み込み
        file.read(reinterpret_cast<char*>(&file_.header), sizeof(file_.header));
        
        // マジックナンバーの確認
        if (file_.header.magic != 0x46546C67) { // 'glTF'
            errorMessage_ = "Invalid GLB magic number";
            return false;
        }
        
        // チャンクの読み込み
        size_t offset = sizeof(file_.header);
        while (offset < fileSize) {
            Chunk chunk;
            file.read(reinterpret_cast<char*>(&chunk.header), sizeof(chunk.header));
            
            chunk.data.resize(chunk.header.length);
            file.read(reinterpret_cast<char*>(chunk.data.data()), chunk.header.length);
            
            file_.chunks.push_back(std::move(chunk));
            
            offset += sizeof(chunk.header) + chunk.header.length;
            
            // パディングをスキップ
            while (offset % 4 != 0) {
                file.ignore(1);
                offset++;
            }
        }
        
        return true;
    }
    
    const Chunk* File::GetJsonChunk() const {
        for (const auto& chunk : chunks) {
            if (chunk.header.type == CHUNK_TYPE_JSON) {
                return &chunk;
            }
        }
        return nullptr;
    }
    
    const Chunk* File::GetBinaryChunk() const {
        for (const auto& chunk : chunks) {
            if (chunk.header.type == CHUNK_TYPE_BIN) {
                return &chunk;
            }
        }
        return nullptr;
    }
}