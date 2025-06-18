// src/Engine/Graphics/Model.cpp
#include "Model.h"
#include "TextureManager.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <unordered_map>
#include <cmath>
#include <functional>

// tinygltf implementation
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// Disable warnings for external library
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4189) // local variable is initialized but not referenced
#pragma warning(disable: 4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4267) // conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable: 4996) // deprecated functions
#endif
#include "../../../externals/tinygltf/tiny_gltf.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

Model::Model() : dxCommon_(nullptr) {}

Model::~Model() {
    // 頂点リソースの解放（Unmapは不要 - 頂点データは永続的にマップされていない）
    if (vertexResource_) {
        vertexResource_.Reset();
    }
}

void Model::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;
}

void Model::LoadFromObj(const std::string& directoryPath, const std::string& filename) {
    // モデルデータの読み込み
    modelData_ = LoadObjFile(directoryPath, filename);

    // モデルデータを最適化（UV球などの表示品質向上のため）
    // ファイル名も渡すように修正
    OptimizeTriangles(modelData_, filename);

    // テクスチャの読み込み
    if (!modelData_.material.textureFilePath.empty()) {
        // テクスチャパスをログに出力
        OutputDebugStringA(("Model: Texture path from MTL: " + modelData_.material.textureFilePath + "\n").c_str());

        // テクスチャが存在するかチェック
        DWORD fileAttributes = GetFileAttributesA(modelData_.material.textureFilePath.c_str());
        if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
            // テクスチャが存在する場合のみ読み込み
            TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
            OutputDebugStringA(("Model: Texture loaded - " + modelData_.material.textureFilePath + "\n").c_str());
        }
        else {
            // テクスチャが見つからない場合、別の場所を探す
            OutputDebugStringA(("WARNING: Texture file not found at: " + modelData_.material.textureFilePath + "\n").c_str());

            // ファイル名のみを抽出
            std::string filenameOnly = modelData_.material.textureFilePath;
            size_t lastSlash = filenameOnly.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                filenameOnly = filenameOnly.substr(lastSlash + 1);
            }

            // 複数の可能性のある場所を探索
            std::vector<std::string> possiblePaths = {
                "Resources/textures/" + filenameOnly,
                directoryPath + "/" + filenameOnly,
                "Resources/" + filenameOnly,
                "Resources/models/" + filenameOnly
            };

            bool found = false;
            for (const auto& path : possiblePaths) {
                OutputDebugStringA(("Model: Trying alternative path: " + path + "\n").c_str());
                if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    // 見つかった場合はパスを更新して読み込み
                    modelData_.material.textureFilePath = path;
                    TextureManager::GetInstance()->LoadTexture(path);
                    OutputDebugStringA(("Model: Texture found and loaded from: " + path + "\n").c_str());
                    found = true;
                    break;
                }
            }

            if (!found) {
                // どこにも見つからない場合
                OutputDebugStringA("WARNING: Texture file not found in any location. Clearing texture path.\n");
                modelData_.material.textureFilePath = ""; // パスをクリア
            }
        }
    }
    else {
        OutputDebugStringA("Model: No texture specified in MTL file\n");
    }

    // 頂点バッファの作成
    vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

    // 頂点バッファビューの設定
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    // 頂点データの書き込み
    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
    vertexResource_->Unmap(0, nullptr);

    // デバッグ情報
    OutputDebugStringA(("Model: Loaded " + std::to_string(modelData_.vertices.size()) + " vertices from " + filename + "\n").c_str());
}

// UV球などの表示品質を向上させるためのモデルデータ最適化関数
void Model::OptimizeTriangles(ModelData& modelData, const std::string& filename) {
    // 最適化前の頂点数を保存
    size_t originalVertexCount = modelData.vertices.size();

    // 重複頂点の検出と削除のためのデータ構造
    std::vector<VertexData> optimizedVertices;
    std::vector<uint32_t> indices;
    std::unordered_map<std::string, uint32_t> vertexMap;

    // 各頂点を処理
    for (const auto& vertex : modelData.vertices) {
        // 頂点のハッシュキーを作成（類似頂点の統合を強化）
        // 精度を大幅に下げて頂点統合を促進（小数点以下2桁に制限）
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
            vertex.position.x, vertex.position.y, vertex.position.z,
            vertex.normal.x, vertex.normal.y, vertex.normal.z,
            vertex.texcoord.x, vertex.texcoord.y);
        std::string key = buffer;

        // この頂点がまだ追加されていなければ追加する
        if (vertexMap.find(key) == vertexMap.end()) {
            vertexMap[key] = static_cast<uint32_t>(optimizedVertices.size());

            // 法線を正規化して品質を向上
            VertexData normalizedVertex = vertex;
            float length = std::sqrt(
                normalizedVertex.normal.x * normalizedVertex.normal.x +
                normalizedVertex.normal.y * normalizedVertex.normal.y +
                normalizedVertex.normal.z * normalizedVertex.normal.z
            );

            if (length > 0.0001f) {
                normalizedVertex.normal.x /= length;
                normalizedVertex.normal.y /= length;
                normalizedVertex.normal.z /= length;
            }

            optimizedVertices.push_back(normalizedVertex);
        }

        // インデックスを追加
        indices.push_back(vertexMap[key]);
    }

    // 法線平均化処理を追加（共有頂点の法線を平均化して滑らかにする）
    std::vector<Vector3> smoothedNormals(optimizedVertices.size(), { 0.0f, 0.0f, 0.0f });
    std::vector<int> normalCount(optimizedVertices.size(), 0);

    // 各三角形の法線を集計
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 < indices.size()) {
            // 三角形の頂点インデックス
            uint32_t idx0 = indices[i];
            uint32_t idx1 = indices[i + 1];
            uint32_t idx2 = indices[i + 2];

            // 三角形の辺ベクトル
            Vector3 edge1 = {
                optimizedVertices[idx1].position.x - optimizedVertices[idx0].position.x,
                optimizedVertices[idx1].position.y - optimizedVertices[idx0].position.y,
                optimizedVertices[idx1].position.z - optimizedVertices[idx0].position.z
            };

            Vector3 edge2 = {
                optimizedVertices[idx2].position.x - optimizedVertices[idx0].position.x,
                optimizedVertices[idx2].position.y - optimizedVertices[idx0].position.y,
                optimizedVertices[idx2].position.z - optimizedVertices[idx0].position.z
            };

            // 外積で面法線を計算
            Vector3 faceNormal = {
                edge1.y * edge2.z - edge1.z * edge2.y,
                edge1.z * edge2.x - edge1.x * edge2.z,
                edge1.x * edge2.y - edge1.y * edge2.x
            };

            // 法線の長さを計算
            float length = std::sqrt(
                faceNormal.x * faceNormal.x +
                faceNormal.y * faceNormal.y +
                faceNormal.z * faceNormal.z
            );

            // 法線を正規化
            if (length > 0.0001f) {
                faceNormal.x /= length;
                faceNormal.y /= length;
                faceNormal.z /= length;

                // 各頂点に面法線を加算
                smoothedNormals[idx0].x += faceNormal.x;
                smoothedNormals[idx0].y += faceNormal.y;
                smoothedNormals[idx0].z += faceNormal.z;
                normalCount[idx0]++;

                smoothedNormals[idx1].x += faceNormal.x;
                smoothedNormals[idx1].y += faceNormal.y;
                smoothedNormals[idx1].z += faceNormal.z;
                normalCount[idx1]++;

                smoothedNormals[idx2].x += faceNormal.x;
                smoothedNormals[idx2].y += faceNormal.y;
                smoothedNormals[idx2].z += faceNormal.z;
                normalCount[idx2]++;
            }
        }
    }

    // 法線を平均化
    for (size_t i = 0; i < optimizedVertices.size(); i++) {
        if (normalCount[i] > 0) {
            smoothedNormals[i].x /= normalCount[i];
            smoothedNormals[i].y /= normalCount[i];
            smoothedNormals[i].z /= normalCount[i];

            // 長さを正規化
            float length = std::sqrt(
                smoothedNormals[i].x * smoothedNormals[i].x +
                smoothedNormals[i].y * smoothedNormals[i].y +
                smoothedNormals[i].z * smoothedNormals[i].z
            );

            if (length > 0.0001f) {
                smoothedNormals[i].x /= length;
                smoothedNormals[i].y /= length;
                smoothedNormals[i].z /= length;
            }

            // 平滑化された法線を適用
            optimizedVertices[i].normal = smoothedNormals[i];
        }
    }

    // UV球の場合は特別な処理（UV座標から理論的な法線を計算）
    bool isSphere = (filename.find("sphere") != std::string::npos) ||
        (filename.find("ball") != std::string::npos) ||
        (filename.find("globe") != std::string::npos);

    if (isSphere) {
        for (size_t i = 0; i < optimizedVertices.size(); i++) {
            // UV座標から球面上の位置を計算
            float u = optimizedVertices[i].texcoord.x;
            float v = optimizedVertices[i].texcoord.y;

            // 球面座標
            float phi = u * 2.0f * 3.14159265f;  // 0～2π
            float theta = v * 3.14159265f;       // 0～π

            // 球面座標から理論的な法線を計算
            Vector3 theoreticalNormal;
            theoreticalNormal.x = std::sin(theta) * std::cos(phi);
            theoreticalNormal.y = std::cos(theta);
            theoreticalNormal.z = std::sin(theta) * std::sin(phi);

            // 99%理論的な法線を使用して完全な球面を強制
            Vector3 blendedNormal;
            blendedNormal.x = 0.99f * theoreticalNormal.x + 0.01f * optimizedVertices[i].normal.x;
            blendedNormal.y = 0.99f * theoreticalNormal.y + 0.01f * optimizedVertices[i].normal.y;
            blendedNormal.z = 0.99f * theoreticalNormal.z + 0.01f * optimizedVertices[i].normal.z;

            // 長さを正規化
            float length = std::sqrt(
                blendedNormal.x * blendedNormal.x +
                blendedNormal.y * blendedNormal.y +
                blendedNormal.z * blendedNormal.z
            );

            if (length > 0.0001f) {
                blendedNormal.x /= length;
                blendedNormal.y /= length;
                blendedNormal.z /= length;
            }

            // 混合された法線を適用
            optimizedVertices[i].normal = blendedNormal;
        }
    }

    // インデックスリストから頂点配列を再構築
    std::vector<VertexData> rebuiltVertices;
    for (size_t i = 0; i < indices.size(); i += 3) {
        // 三角形の頂点を追加（順序を維持）
        if (i + 2 < indices.size()) {
            rebuiltVertices.push_back(optimizedVertices[indices[i + 2]]);
            rebuiltVertices.push_back(optimizedVertices[indices[i + 1]]);
            rebuiltVertices.push_back(optimizedVertices[indices[i]]);
        }
    }

    // 最適化された頂点配列で元の配列を置き換え
    if (!rebuiltVertices.empty()) {
        modelData.vertices = rebuiltVertices;
    }
}

ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    ModelData modelData; // 構築するModelData
    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ1行を格納するもの

    // ファイル読み込み
    std::ifstream file(directoryPath + "/" + filename); // fileを開く
    assert(file.is_open()); // 開けなかったら止める

    OutputDebugStringA(("Model: Loading OBJ file: " + directoryPath + "/" + filename + "\n").c_str());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier; // 先頭の識別子を読む

        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.w = 1.0f;
            position.x *= -1;
            positions.push_back(position);
        }
        else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y = 1 - texcoord.y;
            texcoords.push_back(texcoord);
        }
        else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1;
            normals.push_back(normal);
        }
        else if (identifier == "f") {
            VertexData triangle[3];
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは「位置・UV・法線」で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                // 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];

                triangle[faceVertex] = { position, texcoord, normal };
            }
            // 頂点を逆順で登録することで、周り順を逆にする
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        }
        else if (identifier == "mtllib") {
            // materialTemplateLibraryファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;

            // MTLファイル名をログに出力
            OutputDebugStringA(("Model: Found MTL reference: " + materialFilename + "\n").c_str());

            // 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    return modelData;
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    MaterialData materialData; // 構築するMaterialData
    std::string line; // ファイルから読んだ1行を格納するもの

    // ファイルのフルパス
    std::string mtlPath = directoryPath + "/" + filename;
    std::ifstream file(mtlPath); // ファイルを開く

    // ファイルが開けなかった場合は警告を出力して、デフォルト値を返す
    if (!file.is_open()) {
        OutputDebugStringA(("WARNING: Failed to open MTL file - " + mtlPath + "\n").c_str());
        return materialData;
    }

    OutputDebugStringA(("Model: Successfully opened MTL file - " + mtlPath + "\n").c_str());

    while (std::getline(file, line)) {
        std::string identifier;
        std::stringstream s(line);
        s >> identifier;

        // identifierの応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;

            // テクスチャファイル名をログに出力
            OutputDebugStringA(("MTL Parser: Found texture reference: " + textureFilename + "\n").c_str());

            // 連結してファイルパスにする
            materialData.textureFilePath = directoryPath + "/" + textureFilename;

            // フルパスをログに出力
            OutputDebugStringA(("MTL Parser: Full texture path constructed: " + materialData.textureFilePath + "\n").c_str());
        }
        else if (identifier == "Ka") {
            // ambient color
            s >> materialData.ambient.x >> materialData.ambient.y >> materialData.ambient.z;
            materialData.ambient.w = 1.0f;
        }
        else if (identifier == "Kd") {
            // diffuse color
            s >> materialData.diffuse.x >> materialData.diffuse.y >> materialData.diffuse.z;
            materialData.diffuse.w = 1.0f;
        }
        else if (identifier == "Ks") {
            // specular color
            s >> materialData.specular.x >> materialData.specular.y >> materialData.specular.z;
            materialData.specular.w = 1.0f;
        }
        else if (identifier == "Ns") {
            // shininess
            s >> materialData.shininess;
        }
        else if (identifier == "d" || identifier == "Tr") {
            // transparency (d) or transparency inverted (Tr)
            if (identifier == "d") {
                s >> materialData.alpha;
            }
            else { // Tr (transparency inverted)
                float tr;
                s >> tr;
                materialData.alpha = 1.0f - tr;
            }
        }
    }

    return materialData;
}

void Model::LoadFromGLB(const std::string& filePath) {
    // GLBモデルデータの読み込み
    modelData_ = LoadGLBFile(filePath);
    
    OutputDebugStringA(("Model::LoadFromGLB - Loaded " + std::to_string(modelData_.vertices.size()) + " vertices\n").c_str());
    
    // テクスチャの読み込み（埋め込みテクスチャが保存された後）
    if (!modelData_.material.textureFilePath.empty()) {
        OutputDebugStringA(("Model::LoadFromGLB - Loading texture: " + modelData_.material.textureFilePath + "\n").c_str());
        
        // テクスチャが存在するかチェック
        DWORD fileAttributes = GetFileAttributesA(modelData_.material.textureFilePath.c_str());
        if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
            // テクスチャが存在する場合のみ読み込み
            TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
            OutputDebugStringA(("Model::LoadFromGLB - Texture loaded: " + modelData_.material.textureFilePath + "\n").c_str());
        } else {
            OutputDebugStringA(("WARNING: Texture file not found: " + modelData_.material.textureFilePath + "\n").c_str());
        }
    }
    
    // 頂点バッファの作成
    CreateVertexBuffer();
}

void Model::CreateVertexBuffer() {
    if (modelData_.vertices.empty()) {
        return;
    }
    
    // 頂点バッファの作成
    vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
    
    // 頂点バッファビューの設定
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    
    // 頂点データの書き込み
    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
    vertexResource_->Unmap(0, nullptr);
    
    // デバッグ情報
    OutputDebugStringA(("Model: Created vertex buffer with " + std::to_string(modelData_.vertices.size()) + " vertices\n").c_str());
}

ModelData Model::LoadGLBFile(const std::string& filePath) {
    ModelData result = {};
    
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    
    // GLBファイルを読み込む
    OutputDebugStringA(("Loading GLB file: " + filePath + "\n").c_str());
    
    // ファイルの存在確認
    DWORD fileAttributes = GetFileAttributesA(filePath.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        OutputDebugStringA(("ERROR: GLB file not found: " + filePath + "\n").c_str());
        return result;
    }
    
    bool success = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filePath);
    
    if (!warn.empty()) {
        OutputDebugStringA(("GLTF Warning: " + warn + "\n").c_str());
    }
    
    if (!err.empty()) {
        OutputDebugStringA(("GLTF Error: " + err + "\n").c_str());
    }
    
    if (!success) {
        OutputDebugStringA("Failed to load GLB file\n");
        return result;
    }
    
    OutputDebugStringA(("GLB file loaded successfully. Scenes: " + std::to_string(gltfModel.scenes.size()) + 
                       ", Meshes: " + std::to_string(gltfModel.meshes.size()) + 
                       ", Materials: " + std::to_string(gltfModel.materials.size()) + 
                       ", Textures: " + std::to_string(gltfModel.textures.size()) + "\n").c_str());
    
    // デフォルトシーンを取得
    if (gltfModel.defaultScene < 0 || gltfModel.defaultScene >= static_cast<int>(gltfModel.scenes.size())) {
        OutputDebugStringA("No default scene found in GLB file, using scene 0\n");
        // デフォルトシーンがない場合は最初のシーンを使用
        if (!gltfModel.scenes.empty()) {
            gltfModel.defaultScene = 0;
        } else {
            OutputDebugStringA("ERROR: No scenes found in GLB file\n");
            return result;
        }
    }
    
    // ノード変換行列を計算する関数
    std::function<Matrix4x4(int)> getNodeTransform;
    getNodeTransform = [&](int nodeIdx) -> Matrix4x4 {
        if (nodeIdx < 0 || nodeIdx >= static_cast<int>(gltfModel.nodes.size())) {
            return MakeIdentity4x4();
        }
        
        const tinygltf::Node& node = gltfModel.nodes[nodeIdx];
        Matrix4x4 localTransform = MakeIdentity4x4();
        
        // 変換行列が直接指定されている場合
        if (node.matrix.size() == 16) {
            // Column-major to row-major conversion
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    localTransform.m[i][j] = static_cast<float>(node.matrix[j * 4 + i]);
                }
            }
        } else {
            // TRS (Translation, Rotation, Scale) から行列を構築
            Vector3 translation = {0.0f, 0.0f, 0.0f};
            Vector3 scale = {1.0f, 1.0f, 1.0f};
            Vector4 rotation = {0.0f, 0.0f, 0.0f, 1.0f}; // Quaternion as Vector4 (x,y,z,w)
            
            if (node.translation.size() == 3) {
                translation.x = static_cast<float>(node.translation[0]);
                translation.y = static_cast<float>(node.translation[1]);
                translation.z = static_cast<float>(node.translation[2]);
            }
            
            if (node.scale.size() == 3) {
                scale.x = static_cast<float>(node.scale[0]);
                scale.y = static_cast<float>(node.scale[1]);
                scale.z = static_cast<float>(node.scale[2]);
            }
            
            if (node.rotation.size() == 4) {
                rotation.x = static_cast<float>(node.rotation[0]);
                rotation.y = static_cast<float>(node.rotation[1]);
                rotation.z = static_cast<float>(node.rotation[2]);
                rotation.w = static_cast<float>(node.rotation[3]);
            }
            
            // TRSから行列を作成
            Matrix4x4 T = MakeTranslateMatrix(translation);
            
            // クォータニオンから回転行列を作成
            Matrix4x4 R = MakeIdentity4x4();
            float x = rotation.x, y = rotation.y, z = rotation.z, w = rotation.w;
            R.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
            R.m[0][1] = 2.0f * (x * y - w * z);
            R.m[0][2] = 2.0f * (x * z + w * y);
            R.m[1][0] = 2.0f * (x * y + w * z);
            R.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
            R.m[1][2] = 2.0f * (y * z - w * x);
            R.m[2][0] = 2.0f * (x * z - w * y);
            R.m[2][1] = 2.0f * (y * z + w * x);
            R.m[2][2] = 1.0f - 2.0f * (x * x + y * y);
            
            Matrix4x4 S = MakeScaleMatrix(scale);
            
            // 正しい順序で合成: T * R * S
            localTransform = Multiply(T, Multiply(R, S));
        }
        
        return localTransform;
    };
    
    // シーン内の最初のノードを探す
    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene];
    OutputDebugStringA(("Scene has " + std::to_string(scene.nodes.size()) + " nodes\n").c_str());
    
    Matrix4x4 nodeTransform = MakeIdentity4x4();
    int meshIndex = -1;
    
    // メッシュを持つノードを探す
    for (int nodeIdx : scene.nodes) {
        const tinygltf::Node& node = gltfModel.nodes[nodeIdx];
        OutputDebugStringA(("Checking node " + std::to_string(nodeIdx) + ": " + node.name + ", mesh index: " + std::to_string(node.mesh) + "\n").c_str());
        
        if (node.mesh >= 0) {
            meshIndex = node.mesh;
            nodeTransform = getNodeTransform(nodeIdx);
            OutputDebugStringA(("Found mesh in node: " + node.name + "\n").c_str());
            OutputDebugStringA(("Node transform scale: " + 
                std::to_string(nodeTransform.m[0][0]) + ", " +
                std::to_string(nodeTransform.m[1][1]) + ", " +
                std::to_string(nodeTransform.m[2][2]) + "\n").c_str());
            break;
        }
    }
    
    // シーンにノードがない場合、メッシュを直接探す
    if (meshIndex < 0 && !gltfModel.meshes.empty()) {
        OutputDebugStringA("No mesh found in scene nodes, using first mesh directly\n");
        meshIndex = 0;
    }
    
    if (meshIndex < 0 || meshIndex >= static_cast<int>(gltfModel.meshes.size())) {
        OutputDebugStringA("ERROR: No valid mesh found in GLB file\n");
        return result;
    }
    
    const tinygltf::Mesh& mesh = gltfModel.meshes[meshIndex];
    OutputDebugStringA(("Processing mesh: " + mesh.name + " with " + std::to_string(mesh.primitives.size()) + " primitives\n").c_str());
    
    // 全てのプリミティブを処理
    if (mesh.primitives.empty()) {
        OutputDebugStringA("No primitives found in mesh\n");
        return result;
    }
    
    // デフォルトマテリアルの設定（全体で共通）
    result.material.diffuse = Vector4(0.8f, 0.8f, 0.8f, 1.0f);  // 薄いグレー
    result.material.alpha = 1.0f;  // 不透明
    result.material.ambient = Vector4(0.3f, 0.3f, 0.3f, 1.0f);
    result.material.specular = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
    result.material.shininess = 30.0f;
    
    // 各プリミティブを処理
    for (size_t primIdx = 0; primIdx < mesh.primitives.size(); ++primIdx) {
        const tinygltf::Primitive& primitive = mesh.primitives[primIdx];
        OutputDebugStringA(("Processing primitive " + std::to_string(primIdx) + "\n").c_str());
    
    // 頂点データの取得
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::vector<uint32_t> indices;
    
    // POSITION属性を取得
    auto posIt = primitive.attributes.find("POSITION");
    if (posIt != primitive.attributes.end()) {
        const tinygltf::Accessor& accessor = gltfModel.accessors[posIt->second];
        const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
        
        const float* posData = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        
        for (size_t i = 0; i < accessor.count; ++i) {
            Vector3 pos;
            pos.x = posData[i * 3 + 0];
            pos.y = posData[i * 3 + 1];
            pos.z = posData[i * 3 + 2];
            positions.push_back(pos);
        }
    }
    
    // NORMAL属性を取得
    auto normIt = primitive.attributes.find("NORMAL");
    if (normIt != primitive.attributes.end()) {
        const tinygltf::Accessor& accessor = gltfModel.accessors[normIt->second];
        const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
        
        const float* normData = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        
        for (size_t i = 0; i < accessor.count; ++i) {
            Vector3 norm;
            norm.x = normData[i * 3 + 0];
            norm.y = normData[i * 3 + 1];
            norm.z = normData[i * 3 + 2];
            normals.push_back(norm);
        }
    }
    
    // TEXCOORD_0属性を取得
    auto texIt = primitive.attributes.find("TEXCOORD_0");
    if (texIt != primitive.attributes.end()) {
        const tinygltf::Accessor& accessor = gltfModel.accessors[texIt->second];
        const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
        
        const float* texData = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        
        for (size_t i = 0; i < accessor.count; ++i) {
            Vector2 tex;
            tex.x = texData[i * 2 + 0];
            tex.y = texData[i * 2 + 1];
            texcoords.push_back(tex);
        }
    }
    
    // インデックスの取得
    if (primitive.indices >= 0) {
        const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
        const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
        
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            const uint16_t* indexData = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            for (size_t i = 0; i < accessor.count; ++i) {
                indices.push_back(static_cast<uint32_t>(indexData[i]));
            }
        } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            const uint32_t* indexData = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            for (size_t i = 0; i < accessor.count; ++i) {
                indices.push_back(indexData[i]);
            }
        }
    }
    
    // 法線変換用の行列を計算（スケールの影響を除去）
    Matrix4x4 normalTransform = nodeTransform;
    // 正規化してスケールの影響を除去
    for (int i = 0; i < 3; ++i) {
        float length = std::sqrt(
            normalTransform.m[i][0] * normalTransform.m[i][0] +
            normalTransform.m[i][1] * normalTransform.m[i][1] +
            normalTransform.m[i][2] * normalTransform.m[i][2]
        );
        if (length > 0.0f) {
            normalTransform.m[i][0] /= length;
            normalTransform.m[i][1] /= length;
            normalTransform.m[i][2] /= length;
        }
    }
    
    // デバッグ出力
    OutputDebugStringA(("Vertex data: positions=" + std::to_string(positions.size()) + 
                       ", normals=" + std::to_string(normals.size()) + 
                       ", texcoords=" + std::to_string(texcoords.size()) + 
                       ", indices=" + std::to_string(indices.size()) + "\n").c_str());

    // 頂点データの構築
    if (!indices.empty()) {
        OutputDebugStringA("Building vertex data from indices\n");
        // インデックスを使用して頂点データを構築（3つずつ処理して面の向きを反転）
        for (size_t i = 0; i < indices.size(); i += 3) {
            // 三角形の頂点を逆順で追加（右手座標系から左手座標系への変換）
            for (int j = 2; j >= 0; j--) {
                if (i + j >= indices.size()) continue;
                uint32_t idx = indices[i + j];
            VertexData vertex = {};
            
            if (idx < positions.size()) {
                // 頂点位置にノード変換を適用（X軸を反転して左手座標系に変換）
                Vector4 pos = Vector4(-positions[idx].x, positions[idx].y, positions[idx].z, 1.0f);
                // Vector4と行列の乗算
                Vector4 transformedPos;
                transformedPos.x = pos.x * nodeTransform.m[0][0] + pos.y * nodeTransform.m[1][0] + pos.z * nodeTransform.m[2][0] + pos.w * nodeTransform.m[3][0];
                transformedPos.y = pos.x * nodeTransform.m[0][1] + pos.y * nodeTransform.m[1][1] + pos.z * nodeTransform.m[2][1] + pos.w * nodeTransform.m[3][1];
                transformedPos.z = pos.x * nodeTransform.m[0][2] + pos.y * nodeTransform.m[1][2] + pos.z * nodeTransform.m[2][2] + pos.w * nodeTransform.m[3][2];
                transformedPos.w = pos.x * nodeTransform.m[0][3] + pos.y * nodeTransform.m[1][3] + pos.z * nodeTransform.m[2][3] + pos.w * nodeTransform.m[3][3];
                vertex.position = transformedPos;
            }
            
            if (idx < normals.size()) {
                // 法線にノード変換を適用（回転のみ、X軸を反転）
                Vector3 norm = normals[idx];
                Vector4 norm4 = Vector4(-norm.x, norm.y, norm.z, 0.0f);
                // Vector4と行列の乗算
                Vector4 transformedNorm;
                transformedNorm.x = norm4.x * normalTransform.m[0][0] + norm4.y * normalTransform.m[1][0] + norm4.z * normalTransform.m[2][0] + norm4.w * normalTransform.m[3][0];
                transformedNorm.y = norm4.x * normalTransform.m[0][1] + norm4.y * normalTransform.m[1][1] + norm4.z * normalTransform.m[2][1] + norm4.w * normalTransform.m[3][1];
                transformedNorm.z = norm4.x * normalTransform.m[0][2] + norm4.y * normalTransform.m[1][2] + norm4.z * normalTransform.m[2][2] + norm4.w * normalTransform.m[3][2];
                // 法線を正規化
                Vector3 transformedNorm3 = Vector3(transformedNorm.x, transformedNorm.y, transformedNorm.z);
                float length = std::sqrt(transformedNorm3.x * transformedNorm3.x + transformedNorm3.y * transformedNorm3.y + transformedNorm3.z * transformedNorm3.z);
                if (length > 0.0f) {
                    transformedNorm3.x /= length;
                    transformedNorm3.y /= length;
                    transformedNorm3.z /= length;
                }
                vertex.normal = transformedNorm3;
            } else {
                vertex.normal = Vector3(0.0f, 1.0f, 0.0f);  // デフォルト法線
            }
            
            if (idx < texcoords.size()) {
                vertex.texcoord = texcoords[idx];
            } else {
                vertex.texcoord = Vector2(0.0f, 0.0f);  // デフォルトUV
            }
            
            result.vertices.push_back(vertex);
            }
        }
    } else {
        OutputDebugStringA("No indices found, using vertices directly\n");
        // インデックスがない場合は頂点を直接使用
        for (size_t i = 0; i < positions.size(); ++i) {
            VertexData vertex = {};
            
            // 頂点位置にノード変換を適用（X軸を反転して左手座標系に変換）
            Vector4 pos = Vector4(-positions[i].x, positions[i].y, positions[i].z, 1.0f);
            // Vector4と行列の乗算
            Vector4 transformedPos;
            transformedPos.x = pos.x * nodeTransform.m[0][0] + pos.y * nodeTransform.m[1][0] + pos.z * nodeTransform.m[2][0] + pos.w * nodeTransform.m[3][0];
            transformedPos.y = pos.x * nodeTransform.m[0][1] + pos.y * nodeTransform.m[1][1] + pos.z * nodeTransform.m[2][1] + pos.w * nodeTransform.m[3][1];
            transformedPos.z = pos.x * nodeTransform.m[0][2] + pos.y * nodeTransform.m[1][2] + pos.z * nodeTransform.m[2][2] + pos.w * nodeTransform.m[3][2];
            transformedPos.w = pos.x * nodeTransform.m[0][3] + pos.y * nodeTransform.m[1][3] + pos.z * nodeTransform.m[2][3] + pos.w * nodeTransform.m[3][3];
            vertex.position = transformedPos;
            
            if (i < normals.size()) {
                // 法線にノード変換を適用（回転のみ、X軸を反転）
                Vector3 norm = normals[i];
                Vector4 norm4 = Vector4(-norm.x, norm.y, norm.z, 0.0f);
                // Vector4と行列の乗算
                Vector4 transformedNorm;
                transformedNorm.x = norm4.x * normalTransform.m[0][0] + norm4.y * normalTransform.m[1][0] + norm4.z * normalTransform.m[2][0] + norm4.w * normalTransform.m[3][0];
                transformedNorm.y = norm4.x * normalTransform.m[0][1] + norm4.y * normalTransform.m[1][1] + norm4.z * normalTransform.m[2][1] + norm4.w * normalTransform.m[3][1];
                transformedNorm.z = norm4.x * normalTransform.m[0][2] + norm4.y * normalTransform.m[1][2] + norm4.z * normalTransform.m[2][2] + norm4.w * normalTransform.m[3][2];
                // 法線を正規化
                Vector3 transformedNorm3 = Vector3(transformedNorm.x, transformedNorm.y, transformedNorm.z);
                float length = std::sqrt(transformedNorm3.x * transformedNorm3.x + transformedNorm3.y * transformedNorm3.y + transformedNorm3.z * transformedNorm3.z);
                if (length > 0.0f) {
                    transformedNorm3.x /= length;
                    transformedNorm3.y /= length;
                    transformedNorm3.z /= length;
                }
                vertex.normal = transformedNorm3;
            } else {
                vertex.normal = Vector3(0.0f, 1.0f, 0.0f);
            }
            
            if (i < texcoords.size()) {
                vertex.texcoord = texcoords[i];
            } else {
                vertex.texcoord = Vector2(0.0f, 0.0f);
            }
            
            result.vertices.push_back(vertex);
        }
    }
    
    // マテリアルの処理（プリミティブごと）
    if (primitive.material >= 0 && primitive.material < static_cast<int>(gltfModel.materials.size())) {
        const tinygltf::Material& material = gltfModel.materials[primitive.material];
        
        OutputDebugStringA(("Processing material: " + material.name + "\n").c_str());
        OutputDebugStringA(("  Material has " + std::to_string(gltfModel.textures.size()) + " textures\n").c_str());
        OutputDebugStringA(("  Material has " + std::to_string(gltfModel.images.size()) + " images\n").c_str());
        
        // PBRメタリックラフネスモデルのベースカラーを取得
        const auto& pbr = material.pbrMetallicRoughness;
        if (pbr.baseColorFactor.size() >= 4) {
            result.material.diffuse = Vector4(
                static_cast<float>(pbr.baseColorFactor[0]),
                static_cast<float>(pbr.baseColorFactor[1]),
                static_cast<float>(pbr.baseColorFactor[2]),
                static_cast<float>(pbr.baseColorFactor[3])
            );
            // アルファ値も設定
            result.material.alpha = static_cast<float>(pbr.baseColorFactor[3]);
        }
        
        // ベースカラーテクスチャの処理
        if (pbr.baseColorTexture.index >= 0) {
            const tinygltf::Texture& texture = gltfModel.textures[pbr.baseColorTexture.index];
            if (texture.source >= 0) {
                const tinygltf::Image& image = gltfModel.images[texture.source];
                
                // テクスチャファイルのパスを設定
                if (!image.uri.empty()) {
                    // 相対パスの場合、GLBファイルのディレクトリを基準にする
                    std::string directory = filePath;
                    size_t lastSlash = directory.find_last_of("/\\");
                    if (lastSlash != std::string::npos) {
                        directory = directory.substr(0, lastSlash + 1);
                    }
                    result.material.textureFilePath = directory + image.uri;
                } else if (!image.image.empty()) {
                    // 埋め込み画像の場合
                    OutputDebugStringA(("Processing embedded texture in GLB - size: " + std::to_string(image.image.size()) + " bytes\n").c_str());
                    OutputDebugStringA(("Image dimensions: " + std::to_string(image.width) + "x" + std::to_string(image.height) + "\n").c_str());
                    OutputDebugStringA(("Image components: " + std::to_string(image.component) + "\n").c_str());
                    OutputDebugStringA(("Image pixel type: " + std::to_string(image.pixel_type) + "\n").c_str());
                    
                    // mimeTypeを確認
                    std::string extension = ".png";
                    if (image.mimeType == "image/jpeg") {
                        extension = ".jpg";
                    } else if (image.mimeType == "image/png") {
                        extension = ".png";
                    }
                    
                    // 一時ファイル名を生成
                    std::string tempFilename = "Resources/textures/glb_embedded_" + 
                        std::to_string(texture.source) + "_" + 
                        std::to_string(std::hash<std::string>{}(filePath)) + extension;
                    
                    // ファイルが既に存在するかチェック
                    DWORD fileAttributes = GetFileAttributesA(tempFilename.c_str());
                    if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
                        // ファイルが既に存在する場合は再利用
                        result.material.textureFilePath = tempFilename;
                        OutputDebugStringA(("Reusing existing embedded texture: " + tempFilename + "\n").c_str());
                    } else {
                        // ファイルが存在しない場合は保存
                        // 画像がすでにデコードされているかチェック
                        if (image.width > 0 && image.height > 0 && image.component > 0) {
                        // デコード済みの生データをPNG形式で保存
                        int writeResult = stbi_write_png(tempFilename.c_str(), 
                            image.width, image.height, image.component, 
                            image.image.data(), image.width * image.component);
                        
                        if (writeResult != 0) {
                            result.material.textureFilePath = tempFilename;
                            OutputDebugStringA(("Embedded texture saved as PNG to: " + tempFilename + "\n").c_str());
                        } else {
                            OutputDebugStringA("Failed to write embedded texture as PNG\n");
                        }
                    } else {
                        // エンコードされたデータをそのまま保存
                        std::ofstream file(tempFilename, std::ios::binary);
                        if (file.is_open()) {
                            file.write(reinterpret_cast<const char*>(image.image.data()), image.image.size());
                            file.close();
                            
                            result.material.textureFilePath = tempFilename;
                            OutputDebugStringA(("Embedded texture saved directly to: " + tempFilename + "\n").c_str());
                        } else {
                            OutputDebugStringA("Failed to save embedded texture to file\n");
                        }
                    }
                    }
                }
            }
        }
        
        // エミッシブカラー
        if (material.emissiveFactor.size() >= 3) {
            // エミッシブをアンビエントとして使用
            result.material.ambient = Vector4(
                static_cast<float>(material.emissiveFactor[0]),
                static_cast<float>(material.emissiveFactor[1]),
                static_cast<float>(material.emissiveFactor[2]),
                1.0f
            );
        }
    }
    } // プリミティブループの終了
    
    // デバッグ情報の出力
    OutputDebugStringA(("GLB loaded: " + std::to_string(result.vertices.size()) + " vertices\n").c_str());
    OutputDebugStringA(("Material diffuse: " + 
        std::to_string(result.material.diffuse.x) + ", " +
        std::to_string(result.material.diffuse.y) + ", " +
        std::to_string(result.material.diffuse.z) + ", " +
        std::to_string(result.material.diffuse.w) + "\n").c_str());
    OutputDebugStringA(("Material alpha: " + std::to_string(result.material.alpha) + "\n").c_str());
    OutputDebugStringA(("Texture path: " + (result.material.textureFilePath.empty() ? "None" : result.material.textureFilePath) + "\n").c_str());
    
    return result;
}