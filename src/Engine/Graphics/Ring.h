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

/// <summary>
/// Ring（リング）プリミティブクラス
/// エフェクトでよく利用される円を真ん中でくりぬいたような形状
/// </summary>
class Ring {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    Ring();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Ring();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectXCommon</param>
    void Initialize(DirectXCommon* dxCommon);

    /// <summary>
    /// リングを生成
    /// </summary>
    /// <param name="outerRadius">外径（外側の円の半径）</param>
    /// <param name="innerRadius">内径（内側の円の半径）</param>
    /// <param name="kRingDivide">分割数（大きくするほど円に近づく）</param>
    void Generate(float outerRadius = 1.0f, float innerRadius = 0.2f, uint32_t kRingDivide = 32);

    // アクセサ
    const std::vector<VertexData>& GetVertices() const { return vertices_; }
    uint32_t GetVertexCount() const { return static_cast<uint32_t>(vertices_.size()); }
    const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const { return vertexBufferView_; }
    ID3D12Resource* GetVertexResource() const { return vertexResource_.Get(); }

private:
    // 頂点データ
    std::vector<VertexData> vertices_;
    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    // DirectXCommon
    DirectXCommon* dxCommon_;
};