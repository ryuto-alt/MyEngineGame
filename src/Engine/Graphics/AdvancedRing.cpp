// src/Engine/Graphics/AdvancedRing.cpp
#include "AdvancedRing.h"
#include <cassert>
#include <cmath>
#include <numbers>

AdvancedRing::AdvancedRing() : dxCommon_(nullptr) {}

AdvancedRing::~AdvancedRing() {}

void AdvancedRing::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;
}

void AdvancedRing::Generate(
    float outerRadius,
    float innerRadius,
    uint32_t kRingDivide,
    float startAngle,
    float endAngle,
    UVDirection uvDirection,
    const Vector4& outerColor,
    const Vector4& innerColor
) {
    // 頂点データをクリア
    vertices_.clear();
    colorVertices_.clear();

    // 角度範囲の計算
    float angleRange = endAngle - startAngle;
    bool isFullCircle = (std::abs(angleRange - 2.0f * std::numbers::pi_v<float>) < 0.001f);
    
    // 実際の分割数（部分円の場合は調整）
    uint32_t actualDivide = kRingDivide;
    if (!isFullCircle) {
        // 部分円の場合、角度に比例して分割数を調整
        actualDivide = static_cast<uint32_t>(kRingDivide * (angleRange / (2.0f * std::numbers::pi_v<float>)));
        actualDivide = std::max<unsigned int>(actualDivide, 3u);
    }

    const float radianPerDivide = angleRange / static_cast<float>(actualDivide);

    for (uint32_t index = 0; index < actualDivide; ++index) {
        float currentAngle = startAngle + index * radianPerDivide;
        float nextAngle = startAngle + (index + 1) * radianPerDivide;

        float sin = std::sin(currentAngle);
        float cos = std::cos(currentAngle);
        float sinNext = std::sin(nextAngle);
        float cosNext = std::cos(nextAngle);

        // UV座標の計算（方向に応じて切り替え）
        float u, uNext, vOuter, vInner;
        
        if (uvDirection == UVDirection::Horizontal) {
            // 円に沿ってu方向が変化（基準UV）
            u = static_cast<float>(index) / static_cast<float>(actualDivide);
            uNext = static_cast<float>(index + 1) / static_cast<float>(actualDivide);
            vOuter = 0.0f; // 外側
            vInner = 1.0f; // 内側
        } else {
            // v方向に変化
            u = 0.0f; // 固定
            uNext = 0.0f; // 固定
            vOuter = static_cast<float>(index) / static_cast<float>(actualDivide);
            vInner = static_cast<float>(index + 1) / static_cast<float>(actualDivide);
        }

        // ①：外側の現在の頂点
        VertexData outerCurrent;
        outerCurrent.position = {-sin * outerRadius, cos * outerRadius, 0.0f, 1.0f};
        outerCurrent.texcoord = {u, vOuter};
        outerCurrent.normal = {0.0f, 0.0f, 1.0f};

        // ②：外側の次の頂点
        VertexData outerNext;
        outerNext.position = {-sinNext * outerRadius, cosNext * outerRadius, 0.0f, 1.0f};
        outerNext.texcoord = {uNext, vOuter};
        outerNext.normal = {0.0f, 0.0f, 1.0f};

        // ③：内側の現在の頂点
        VertexData innerCurrent;
        innerCurrent.position = {-sin * innerRadius, cos * innerRadius, 0.0f, 1.0f};
        innerCurrent.texcoord = {u, vInner};
        innerCurrent.normal = {0.0f, 0.0f, 1.0f};

        // ④：内側の次の頂点
        VertexData innerNext;
        innerNext.position = {-sinNext * innerRadius, cosNext * innerRadius, 0.0f, 1.0f};
        innerNext.texcoord = {uNext, vInner};
        innerNext.normal = {0.0f, 0.0f, 1.0f};

        // カラー付きバージョンも作成（将来の拡張用）
        VertexDataWithColor outerCurrentColor = {outerCurrent.position, outerCurrent.texcoord, outerCurrent.normal, outerColor};
        VertexDataWithColor outerNextColor = {outerNext.position, outerNext.texcoord, outerNext.normal, outerColor};
        VertexDataWithColor innerCurrentColor = {innerCurrent.position, innerCurrent.texcoord, innerCurrent.normal, innerColor};
        VertexDataWithColor innerNextColor = {innerNext.position, innerNext.texcoord, innerNext.normal, innerColor};

        // 第1三角形: 外側現在 → 内側現在 → 外側次
        vertices_.push_back(outerCurrent);
        vertices_.push_back(innerCurrent);
        vertices_.push_back(outerNext);

        colorVertices_.push_back(outerCurrentColor);
        colorVertices_.push_back(innerCurrentColor);
        colorVertices_.push_back(outerNextColor);

        // 第2三角形: 内側現在 → 内側次 → 外側次
        vertices_.push_back(innerCurrent);
        vertices_.push_back(innerNext);
        vertices_.push_back(outerNext);

        colorVertices_.push_back(innerCurrentColor);
        colorVertices_.push_back(innerNextColor);
        colorVertices_.push_back(outerNextColor);
    }

    // 頂点バッファの作成
    if (!vertices_.empty()) {
        vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertices_.size());

        // 頂点バッファビューの設定
        vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
        vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
        vertexBufferView_.StrideInBytes = sizeof(VertexData);

        // 頂点データの書き込み
        VertexData* vertexData = nullptr;
        vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
        std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
    }
}