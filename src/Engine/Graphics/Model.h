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

    // 基本形状の作成
    void CreateCube(); // 立方体の作成
    void CreateSphere(int segments = 16); // 球体の作成

    // 描画
    void Draw();

    // トランスフォーム設定
    void SetPosition(const Vector3& position) { position_ = position; }
    void SetRotation(const Vector3& rotation) { rotation_ = rotation; }
    void SetScale(const Vector3& scale) { scale_ = scale; }
    void Update(); // トランスフォームを更新

    // アクセサ
    const std::vector<VertexData>& GetVertices() const { return modelData_.vertices; }
    uint32_t GetVertexCount() const { return static_cast<uint32_t>(modelData_.vertices.size()); }
    const MaterialData& GetMaterial() const { return modelData_.material; }
    const std::string& GetTextureFilePath() const { return modelData_.material.textureFilePath; }
    const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const { return vertexBufferView_; }
    ID3D12Resource* GetVertexResource() const { return vertexResource_.Get(); }

private:
    // モデルデータの最適化（UV球など改善のため）
    void OptimizeTriangles(ModelData& modelData, const std::string& filename);

    // モデルデータの読み込み
    ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
    // マテリアルデータの読み込み
    MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

    // クロス積の計算を行うヘルパー関数
    Vector3 CalculateNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);

    // 順序付き頂点リストから三角形を生成するユーティリティ
    void AddTriangle(const Vector3& v1, const Vector3& v2, const Vector3& v3, 
                      const Vector2& uv1, const Vector2& uv2, const Vector2& uv3);

    // モデルデータ
    ModelData modelData_;
    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    // DirectXCommon
    DirectXCommon* dxCommon_;

    // トランスフォーム
    Vector3 position_ = {0.0f, 0.0f, 0.0f};
    Vector3 rotation_ = {0.0f, 0.0f, 0.0f};
    Vector3 scale_ = {1.0f, 1.0f, 1.0f};
    Matrix4x4 worldMatrix_;
};