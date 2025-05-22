// src/Engine/Graphics/Ring.cpp
#include "Ring.h"
#include <cassert>
#include <cmath>
#include <numbers>

Ring::Ring() : dxCommon_(nullptr) {}

Ring::~Ring() {}

void Ring::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;
}

void Ring::Generate(float outerRadius, float innerRadius, uint32_t kRingDivide) {
    // 頂点データをクリア
    vertices_.clear();

    // XY平面上のRingを作成。時計回りに三角形を作っていく
    const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kRingDivide);

    for (uint32_t index = 0; index < kRingDivide; ++index) {
        float sin = std::sin(index * radianPerDivide);
        float cos = std::cos(index * radianPerDivide);
        float sinNext = std::sin((index + 1) * radianPerDivide);
        float cosNext = std::cos((index + 1) * radianPerDivide);

        // UV座標計算用（円方向に展開）
        float u = static_cast<float>(index) / static_cast<float>(kRingDivide);
        float uNext = static_cast<float>(index + 1) / static_cast<float>(kRingDivide);
        
        // position+uv,normal を設定済みにしたデータができる
        // gradationLine.pngを使用するため、UV座標を円形エフェクト用に設定
        // ①：外側の現在の頂点
        VertexData outerCurrent;
        outerCurrent.position = {-sin * outerRadius, cos * outerRadius, 0.0f, 1.0f};
        outerCurrent.texcoord = {u, 0.0f}; // 外側はv=0.0（gradationLine.pngの外端）
        outerCurrent.normal = {0.0f, 0.0f, 1.0f};

        // ②：外側の次の頂点
        VertexData outerNext;
        outerNext.position = {-sinNext * outerRadius, cosNext * outerRadius, 0.0f, 1.0f};
        outerNext.texcoord = {uNext, 0.0f}; // 外側はv=0.0
        outerNext.normal = {0.0f, 0.0f, 1.0f};

        // ③：内側の現在の頂点
        VertexData innerCurrent;
        innerCurrent.position = {-sin * innerRadius, cos * innerRadius, 0.0f, 1.0f};
        innerCurrent.texcoord = {u, 1.0f}; // 内側はv=1.0（gradationLine.pngの内端）
        innerCurrent.normal = {0.0f, 0.0f, 1.0f};

        // ④：内側の次の頂点
        VertexData innerNext;
        innerNext.position = {-sinNext * innerRadius, cosNext * innerRadius, 0.0f, 1.0f};
        innerNext.texcoord = {uNext, 1.0f}; // 内側はv=1.0
        innerNext.normal = {0.0f, 0.0f, 1.0f};

        // 第1三角形: 外側現在 → 内側現在 → 外側次
        vertices_.push_back(outerCurrent);
        vertices_.push_back(innerCurrent);
        vertices_.push_back(outerNext);

        // 第2三角形: 内側現在 → 内側次 → 外側次
        vertices_.push_back(innerCurrent);
        vertices_.push_back(innerNext);
        vertices_.push_back(outerNext);
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