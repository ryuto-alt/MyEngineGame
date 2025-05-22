// ポータルエフェクト - 異次元への扉のような派手なエフェクト
// src/Engine/Graphics/PortalEffect.cpp
#include "PortalEffect.h"
#include "TextureManager.h"
#include "Math.h"
#include <cassert>
#include <algorithm>
#include <cmath>

// 定数定義
namespace {
    constexpr float PI = 3.14159265f;
}

PortalEffect::PortalEffect()
    : dxCommon_(nullptr)
    , spriteCommon_(nullptr)
    , camera_(nullptr)
    , position_{0.0f, 0.0f, 0.0f}
    , scale_(1.0f)
    , intensity_(1.0f)
    , effectTime_(0.0f)
    , isVisible_(true)
    , isActive_(false)
    , portalType_(0) {
}

PortalEffect::~PortalEffect() {}

void PortalEffect::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    assert(dxCommon);
    assert(spriteCommon);
    
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    
    // テクスチャの事前読み込み
    TextureManager::GetInstance()->LoadTexture("Resources/particle/gradationLine.png");
    
    OutputDebugStringA("PortalEffect::Initialize - Portal Effect ready\n");
}

void PortalEffect::CreatePortal(const Vector3& position) {
    position_ = position;
    rings_.clear();
    
    CreatePortalRings();
    StartEffect();
    
    OutputDebugStringA("PortalEffect::CreatePortal - Interdimensional portal created\n");
}

void PortalEffect::CreatePortalRings() {
    rings_.reserve(NUM_RINGS);
    
    for (int i = 0; i < NUM_RINGS; ++i) {
        PortalRing ring;
        
        // 各リングのサイズと深度を計算
        float normalizedIndex = static_cast<float>(i) / static_cast<float>(NUM_RINGS - 1);
        float radius = 0.5f + normalizedIndex * 2.5f; // 0.5 to 3.0
        float innerRadius = radius * 0.7f; // 内径は外径の70%
        
        ring.ring = std::make_unique<Ring>();
        ring.ring->Initialize(dxCommon_);
        ring.ring->Generate(radius, innerRadius, 32 + i * 4); // 奥ほど細かく
        
        ring.object3d = std::make_unique<Object3d>();
        ring.object3d->Initialize(dxCommon_, spriteCommon_);
        ring.object3d->SetEnableLighting(false);
        
        ring.radius = radius;
        ring.depth = -normalizedIndex * DEPTH_RANGE; // 奥行き
        ring.spiralSpeed = SPIRAL_SPEED * (1.0f + normalizedIndex * 0.5f); // 奥ほど高速
        ring.spiralAngle = normalizedIndex * PI * 2.0f; // 初期角度をずらす
        ring.distortionPhase = normalizedIndex * PI;
        ring.fadeIntensity = 1.0f - normalizedIndex * 0.3f; // 奥ほど薄く
        ring.layerIndex = i;
        
        // ポータルタイプに応じた色設定
        if (portalType_ == 0) {
            // 青いポータル（冷たい異次元）
            float blue = 0.2f + normalizedIndex * 0.8f;
            ring.baseColor = {0.1f, 0.3f + normalizedIndex * 0.5f, blue, 0.8f - normalizedIndex * 0.3f};
        } else {
            // 赤いポータル（熱い地獄の扉）
            float red = 0.3f + normalizedIndex * 0.7f;
            ring.baseColor = {red, 0.1f + normalizedIndex * 0.3f, 0.05f, 0.8f - normalizedIndex * 0.3f};
        }
        
        rings_.push_back(std::move(ring));
    }
}

void PortalEffect::Update(float deltaTime) {
    if (!isActive_ || !isVisible_) return;
    
    effectTime_ += deltaTime;
    UpdateRings(deltaTime);
    UpdateDistortion(deltaTime);
}

void PortalEffect::UpdateRings(float deltaTime) {
    for (auto& ring : rings_) {
        // スパイラル回転の更新
        ring.spiralAngle += ring.spiralSpeed * deltaTime;
        
        // 歪み効果の計算
        float distortion = DISTORTION_AMPLITUDE * std::sinf(effectTime_ * 3.0f + ring.distortionPhase);
        float scaleModifier = 1.0f + distortion * 0.1f;
        
        // 渦巻き効果の位置計算
        float spiralRadius = std::sinf(effectTime_ * 2.0f + ring.layerIndex * 0.5f) * 0.2f;
        Vector3 spiralOffset = {
            spiralRadius * std::cosf(ring.spiralAngle),
            spiralRadius * std::sinf(ring.spiralAngle),
            0.0f
        };
        
        // 深度による収束効果
        float depthFactor = 1.0f + ring.depth * 0.1f;
        Vector3 finalPosition = {
            position_.x + spiralOffset.x * depthFactor,
            position_.y + spiralOffset.y * depthFactor,
            position_.z + ring.depth
        };
        
        // 色の強度変化（時間で脈動）
        float intensityModifier = 0.7f + 0.3f * std::sinf(effectTime_ * 4.0f + ring.layerIndex * 0.8f);
        Vector4 currentColor = ring.baseColor;
        currentColor.x *= intensityModifier * intensity_ * ring.fadeIntensity;
        currentColor.y *= intensityModifier * intensity_ * ring.fadeIntensity;
        currentColor.z *= intensityModifier * intensity_ * ring.fadeIntensity;
        currentColor.w *= intensityModifier * ring.fadeIntensity;
        
        // Object3Dの更新
        if (ring.object3d) {
            ring.object3d->SetPosition(finalPosition);
            ring.object3d->SetRotation({0.0f, 0.0f, ring.spiralAngle * 0.5f}); // ゆっくり回転
            ring.object3d->SetScale({
                scale_ * scaleModifier * depthFactor,
                scale_ * scaleModifier * depthFactor,
                scale_
            });
            ring.object3d->SetColor(currentColor);
            
            // 渦巻きUVスクロール
            float uvScrollU = effectTime_ * ring.spiralSpeed * 0.2f;
            float uvScrollV = effectTime_ * 1.5f + ring.layerIndex * 0.3f;
            ring.object3d->SetUVScroll(uvScrollU, uvScrollV);
            
            if (camera_) {
                ring.object3d->SetCamera(camera_);
                ring.object3d->Update();
            }
        }
    }
}

void PortalEffect::UpdateDistortion(float deltaTime) {
    // 全体的な歪み効果をここで追加実装可能
    // 今回は個別リングの歪みで十分な効果を得る
}

void PortalEffect::Draw() {
    if (!isVisible_ || !isActive_) return;
    
    // 奥から手前の順に描画（正しい深度感）
    for (int i = NUM_RINGS - 1; i >= 0; --i) {
        auto& ring = rings_[i];
        if (ring.ring && ring.object3d) {
            assert(dxCommon_);
            
            // 共通描画設定
            spriteCommon_->CommonDraw();
            
            // Ringの頂点バッファをセット
            dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &ring.ring->GetVBView());
            
            // マテリアルCBufferの場所を設定
            dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, 
                ring.object3d->GetMaterialResource()->GetGPUVirtualAddress());
            
            // 変換行列CBufferの場所を設定
            dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, 
                ring.object3d->GetTransformationMatrixResource()->GetGPUVirtualAddress());
            
            // テクスチャの場所を設定
            const std::string texturePath = "Resources/particle/gradationLine.png";
            dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2,
                TextureManager::GetInstance()->GetSrvHandleGPU(texturePath));
            
            // ライトCBufferの場所を設定
            dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, 
                ring.object3d->GetDirectionalLightResource()->GetGPUVirtualAddress());
            
            // 描画
            dxCommon_->GetCommandList()->DrawInstanced(ring.ring->GetVertexCount(), 1, 0, 0);
        }
    }
}

void PortalEffect::SetCamera(Camera* camera) {
    camera_ = camera;
}

void PortalEffect::SetPosition(const Vector3& position) {
    position_ = position;
}

void PortalEffect::SetScale(float scale) {
    scale_ = scale;
}

void PortalEffect::SetIntensity(float intensity) {
    intensity_ = std::clamp(intensity, 0.0f, 2.0f);
}

void PortalEffect::SetPortalType(int type) {
    if (type != portalType_) {
        portalType_ = type;
        // 色を再計算
        if (!rings_.empty()) {
            CreatePortalRings();
        }
        OutputDebugStringA(("PortalEffect::SetPortalType - Portal type changed to: " + 
                          std::to_string(type) + "\n").c_str());
    }
}

void PortalEffect::StartEffect() {
    isActive_ = true;
    effectTime_ = 0.0f;
    OutputDebugStringA("PortalEffect::StartEffect - Interdimensional portal opened!\n");
}

void PortalEffect::StopEffect() {
    isActive_ = false;
    OutputDebugStringA("PortalEffect::StopEffect - Portal closed\n");
}
