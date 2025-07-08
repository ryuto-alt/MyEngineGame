#pragma once
#include <string>
#include <vector>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <d3d12.h>
#include <wrl.h>
#include "DirectXCommon.h"
#include "Mymath.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// モデルデータクラス
class Model {
public:
    // コンストラクタ
    Model();
    // デストラクタ
    ~Model();

    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // モデルの読み込み
    void LoadFromObj(const std::string& directoryPath, const std::string& filename);

    // アクセサ
    const std::vector<VertexData>& GetVertices() const { return modelData_.vertices; }
    const std::vector<uint32_t>& GetIndices() const { return modelData_.indices; }
    uint32_t GetVertexCount() const { return static_cast<uint32_t>(modelData_.vertices.size()); }
    uint32_t GetIndexCount() const { return static_cast<uint32_t>(modelData_.indices.size()); }
    const MaterialData& GetMaterial() const { return modelData_.material; }
    const std::string& GetTextureFilePath() const { return modelData_.material.textureFilePath; }
    const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const { return vertexBufferView_; }
    const D3D12_INDEX_BUFFER_VIEW& GetIBView() const { return indexBufferView_; }
    ID3D12Resource* GetVertexResource() const { return vertexResource_.Get(); }
    ID3D12Resource* GetIndexResource() const { return indexResource_.Get(); }
    const ModelData& GetModelData() const { return modelData_; }

protected:
    // モデルデータアクセサ（継承クラス用）
    ModelData& GetModelDataInternal() { return modelData_; }
    
    // 頂点バッファの作成（継承クラス用）
    void CreateVertexBuffer();
    // インデックスバッファの作成（継承クラス用）
    void CreateIndexBuffer();

private:
    void LoadWithAssimp(const std::string& directoryPath, const std::string& filename);
    void ProcessAssimpScene(const aiScene* scene, const std::string& directoryPath, const std::string& objFileName = "");
    void ProcessAssimpMesh(const aiMesh* mesh, const aiScene* scene);
    void ProcessAssimpMaterial(const aiMaterial* material, const std::string& directoryPath, const std::string& objFileName = "");
    std::string FindTextureInDirectory(const std::string& directoryPath);
    std::string ParseMTLFile(const std::string& directoryPath, const std::string& objFileName);

    // モデルデータ
    ModelData modelData_;
    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    // インデックスバッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    // インデックスバッファビュー
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
    // DirectXCommon
    DirectXCommon* dxCommon_;
    // assimpインポーター
    Assimp::Importer assimpImporter_;
};