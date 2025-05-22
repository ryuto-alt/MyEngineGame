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
/// 拡張Ring（リング）プリミティブクラス
/// エフェクトでよく利用される円を真ん中でくりぬいたような形状の拡張版
/// UVScrollやパーティクルシステムとの組み合わせに対応
/// </summary>
class AdvancedRing {
public:
    /// <summary>
    /// UV方向の列挙
    /// </summary>
    enum class UVDirection {
        Horizontal, // 円に沿ってu方向が変化（デフォルト）
        Vertical    // v方向に変化
    };

    /// <summary>
    /// コンストラクタ
    /// </summary>
    AdvancedRing();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~AdvancedRing();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectXCommon</param>
    void Initialize(DirectXCommon* dxCommon);

    /// <summary>
    /// 拡張リングを生成
    /// </summary>
    /// <param name="outerRadius">外径（外側の円の半径）</param>
    /// <param name="innerRadius">内径（内側の円の半径）</param>
    /// <param name="kRingDivide">分割数（大きくするほど円に近づく）</param>
    /// <param name="startAngle">開始角度（ラジアン）</param>
    /// <param name="endAngle">終了角度（ラジアン）</param>
    /// <param name="uvDirection">UV方向</param>
    /// <param name="outerColor">外側の頂点カラー</param>
    /// <param name="innerColor">内側の頂点カラー</param>
    void Generate(
        float outerRadius = 1.0f,
        float innerRadius = 0.2f,
        uint32_t kRingDivide = 32,
        float startAngle = 0.0f,
        float endAngle = 2.0f * 3.14159265f,
        UVDirection uvDirection = UVDirection::Horizontal,
        const Vector4& outerColor = {1.0f, 1.0f, 1.0f, 1.0f},
        const Vector4& innerColor = {1.0f, 1.0f, 1.0f, 1.0f}
    );

    // アクセサ
    const std::vector<VertexData>& GetVertices() const { return vertices_; }
    uint32_t GetVertexCount() const { return static_cast<uint32_t>(vertices_.size()); }
    const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const { return vertexBufferView_; }
    ID3D12Resource* GetVertexResource() const { return vertexResource_.Get(); }

private:
    // 頂点データ（カラー対応）
    struct VertexDataWithColor {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
        Vector4 color;
    };

    // 頂点データ
    std::vector<VertexData> vertices_;
    // カラー付き頂点データ（将来の拡張用）
    std::vector<VertexDataWithColor> colorVertices_;
    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    // DirectXCommon
    DirectXCommon* dxCommon_;
};