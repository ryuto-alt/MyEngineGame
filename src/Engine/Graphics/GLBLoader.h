#pragma once
#include "GLBStructures.h"
#include "GLTFStructures.h"
#include "JsonParser.h"
#include "Mymath.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// GLBファイルローダークラス
class GLBLoader {
public:
    GLBLoader();
    ~GLBLoader();

    // GLBファイルを読み込んでModelDataに変換
    bool LoadFromFile(const std::string& filePath, ModelData& outModelData);
    
    // GLBファイルを読み込んで複数のModelDataに変換（複数メッシュ対応）
    bool LoadFromFile(const std::string& filePath, std::vector<ModelData>& outModelDataList);
    
    // エラーメッセージの取得
    const std::string& GetErrorMessage() const { return errorMessage_; }
    
    // 読み込み統計の取得（デバッグ用）
    struct LoadStatistics {
        size_t vertexCount = 0;
        size_t triangleCount = 0;
        size_t materialCount = 0;
        size_t textureCount = 0;
        std::string loadTime;
    };
    
    const LoadStatistics& GetLoadStatistics() const { return statistics_; }

private:
    std::string errorMessage_;
    LoadStatistics statistics_;
    GLB::File glbFile_;
    GLTF::Document gltfDocument_;
    std::vector<uint8_t> binaryBuffer_;
    std::string baseDirectory_;
    
    // GLBファイルの解析
    bool ParseGLBFile(const std::string& filePath);
    
    // JSONチャンクからglTFドキュメントを解析
    bool ParseGLTFDocument(const uint8_t* jsonData, size_t jsonSize);
    
    // glTFドキュメントからModelDataリストを生成
    bool ConvertToModelData(std::vector<ModelData>& outModelDataList);
    
    // ノード走査のヘルパー関数
    void TraverseNode(GLTF::Index nodeIndex, std::vector<ModelData>& outModelDataList);
    
    // アクセサからデータを読み取り
    bool ReadAccessorData(const GLTF::Accessor& accessor, std::vector<uint8_t>& outData);
    
    // 頂点データの変換
    bool ConvertVertexData(const GLTF::Primitive& primitive, std::vector<VertexData>& outVertices);
    
    // インデックスデータの変換
    bool ConvertIndexData(const GLTF::Primitive& primitive, std::vector<uint32_t>& outIndices);
    
    // マテリアルデータの変換
    bool ConvertMaterialData(const GLTF::Material& gltfMaterial, MaterialData& outMaterial);
    
    // テクスチャパスの解決
    std::string ResolveTexturePath(int32_t textureIndex);
    
    // JSON解析関連
    bool ParseAsset(const std::shared_ptr<Json::JsonNode>& assetNode, GLTF::Asset& asset);
    bool ParseScenes(const std::shared_ptr<Json::JsonNode>& scenesNode, std::vector<GLTF::Scene>& scenes);
    bool ParseNodes(const std::shared_ptr<Json::JsonNode>& nodesNode, std::vector<GLTF::Node>& nodes);
    bool ParseMeshes(const std::shared_ptr<Json::JsonNode>& meshesNode, std::vector<GLTF::Mesh>& meshes);
    bool ParseMaterials(const std::shared_ptr<Json::JsonNode>& materialsNode, std::vector<GLTF::Material>& materials);
    bool ParseTextures(const std::shared_ptr<Json::JsonNode>& texturesNode, std::vector<GLTF::Texture>& textures);
    bool ParseImages(const std::shared_ptr<Json::JsonNode>& imagesNode, std::vector<GLTF::Image>& images);
    bool ParseSamplers(const std::shared_ptr<Json::JsonNode>& samplersNode, std::vector<GLTF::Sampler>& samplers);
    bool ParseBuffers(const std::shared_ptr<Json::JsonNode>& buffersNode, std::vector<GLTF::Buffer>& buffers);
    bool ParseBufferViews(const std::shared_ptr<Json::JsonNode>& bufferViewsNode, std::vector<GLTF::BufferView>& bufferViews);
    bool ParseAccessors(const std::shared_ptr<Json::JsonNode>& accessorsNode, std::vector<GLTF::Accessor>& accessors);
    
    // ヘルパー関数
    bool ParsePbrMetallicRoughness(const std::shared_ptr<Json::JsonNode>& pbrNode, GLTF::PbrMetallicRoughness& pbr);
    bool ParseTextureInfo(const std::shared_ptr<Json::JsonNode>& textureInfoNode, GLTF::TextureInfo& textureInfo);
    bool ParseAttributes(const std::shared_ptr<Json::JsonNode>& attributesNode, GLTF::Attributes& attributes);
    
    Vector3 ParseVector3(const std::shared_ptr<Json::JsonNode>& arrayNode, const Vector3& defaultValue = {0.0f, 0.0f, 0.0f});
    Vector4 ParseVector4(const std::shared_ptr<Json::JsonNode>& arrayNode, const Vector4& defaultValue = {0.0f, 0.0f, 0.0f, 0.0f});
    Matrix4x4 ParseMatrix4x4(const std::shared_ptr<Json::JsonNode>& arrayNode, const Matrix4x4& defaultValue = MakeIdentity4x4());
    
    // データタイプ変換
    template<typename T>
    bool ExtractTypedData(const uint8_t* data, size_t elementCount, int32_t componentType, bool normalized, std::vector<T>& outData);
    
    // バイト順変換（リトルエンディアン）
    template<typename T>
    T ReadLittleEndian(const uint8_t* data, size_t offset);
    
    // コンポーネントサイズ計算
    size_t GetComponentSize(int32_t componentType);
    size_t GetTypeElementCount(const std::string& type);
    
    // バイナリデータアクセス
    const uint8_t* GetBinaryData(size_t offset, size_t size);
    
    // エラー設定
    void SetError(const std::string& message);
    
    // ログ出力
    void LogInfo(const std::string& message);
    void LogWarning(const std::string& message);
};