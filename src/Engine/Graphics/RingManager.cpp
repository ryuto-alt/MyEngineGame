// src/Engine/Graphics/RingManager.cpp
#include "RingManager.h"
#include <cassert>
#include <numbers>

RingManager* RingManager::GetInstance() {
    static RingManager instance;
    return &instance;
}

void RingManager::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;
}

Ring* RingManager::CreateRing(
    const std::string& name,
    float outerRadius,
    float innerRadius,
    uint32_t divisions
) {
    assert(dxCommon_);

    // 既に存在する場合は削除
    if (rings_.find(name) != rings_.end()) {
        rings_.erase(name);
    }

    // 新しいRingを作成
    auto ring = std::make_unique<Ring>();
    ring->Initialize(dxCommon_);
    ring->Generate(outerRadius, innerRadius, divisions);

    Ring* ringPtr = ring.get();
    rings_[name] = std::move(ring);

    return ringPtr;
}

AdvancedRing* RingManager::CreateAdvancedRing(
    const std::string& name,
    float outerRadius,
    float innerRadius,
    uint32_t divisions,
    float startAngle,
    float endAngle,
    AdvancedRing::UVDirection uvDirection,
    const Vector4& outerColor,
    const Vector4& innerColor
) {
    assert(dxCommon_);

    // 既に存在する場合は削除
    if (advancedRings_.find(name) != advancedRings_.end()) {
        advancedRings_.erase(name);
    }

    // 新しいAdvancedRingを作成
    auto ring = std::make_unique<AdvancedRing>();
    ring->Initialize(dxCommon_);
    ring->Generate(outerRadius, innerRadius, divisions, startAngle, endAngle, uvDirection, outerColor, innerColor);

    AdvancedRing* ringPtr = ring.get();
    advancedRings_[name] = std::move(ring);

    return ringPtr;
}

Ring* RingManager::GetRing(const std::string& name) {
    auto it = rings_.find(name);
    if (it != rings_.end()) {
        return it->second.get();
    }
    return nullptr;
}

AdvancedRing* RingManager::GetAdvancedRing(const std::string& name) {
    auto it = advancedRings_.find(name);
    if (it != advancedRings_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void RingManager::CreateCommonPresets() {
    assert(dxCommon_);

    // よく使用されるRingプリセットを作成

    // 1. 基本的なエフェクト用Ring
    CreateRing("effect_basic", 1.0f, 0.2f, 32);

    // 2. 細いRing（境界線効果）
    CreateRing("effect_thin", 1.0f, 0.9f, 64);

    // 3. 太いRing（大きなエフェクト）
    CreateRing("effect_thick", 1.0f, 0.1f, 32);

    // 4. 高解像度Ring（滑らかなアニメーション用）
    CreateRing("effect_smooth", 1.0f, 0.2f, 128);

    // 高機能Ring

    // 5. 半円Ring（爆発エフェクト用）
    CreateAdvancedRing("effect_semicircle", 
        1.0f, 0.2f, 32,
        0.0f, std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f});

    // 6. 縦スクロール用Ring
    CreateAdvancedRing("effect_vertical_scroll",
        1.0f, 0.2f, 32,
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Vertical,
        {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f});

    // 7. グラデーション効果Ring（外側から内側への色変化）
    CreateAdvancedRing("effect_gradient",
        1.0f, 0.2f, 32,
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}); // 赤から青へ

    // 8. 部分円Ring（スライス効果）
    CreateAdvancedRing("effect_slice",
        1.0f, 0.2f, 16,
        0.0f, std::numbers::pi_v<float> / 2.0f, // 90度
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
}

void RingManager::Clear() {
    rings_.clear();
    advancedRings_.clear();
}