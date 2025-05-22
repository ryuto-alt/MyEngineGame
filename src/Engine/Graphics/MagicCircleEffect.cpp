// 魔法陣エフェクト - ゲーム品質の派手なエフェクト
// src/Engine/Graphics/MagicCircleEffect.cpp
#include "MagicCircleEffect.h"
#include "TextureManager.h"
#include "Math.h"
#include <cassert>
#include <algorithm>
#include <cmath>

// 定数定義
namespace {
    constexpr float PI = 3.14159265f;
}

MagicCircleEffect::MagicCircleEffect()
    : dxCommon_(nullptr)
    , spriteCommon_(nullptr)
    , camera_(nullptr)
    , position_{0.0f, 0.0f, 0.0f}
    , scale_(1.0f)
    , intensity_(1.0f)
    , effectTime_(0.0f)
    , isVisible_(true)
    , isActive_(false) {
}

MagicCircleEffect::~MagicCircleEffect() {}

void MagicCircleEffect::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    assert(dxCommon);
    assert(spriteCommon);
    
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    
    // テクスチャの事前読み込み
    TextureManager::GetInstance()->LoadTexture("Resources/particle/gradationLine.png");
    
    OutputDebugStringA("MagicCircleEffect::Initialize - Magic Circle Effect ready\n");
}

void MagicCircleEffect::CreateMagicCircle(const Vector3& position) {
    position_ = position;
    layers_.clear();
    
    CreateLayers();
    StartEffect();
    
    OutputDebugStringA("MagicCircleEffect::CreateMagicCircle - Magic circle created with multiple layers\n");
}

void MagicCircleEffect::CreateLayers() {
    layers_.reserve(NUM_LAYERS);
    
    // レイヤー1: 外側の大きな魔法円（ゆっくり回転）
    {
        MagicLayer layer;
        layer.ring = std::make_unique<Ring>();
        layer.ring->Initialize(dxCommon_);
        layer.ring->Generate(4.0f, 3.6f, 64); // 大きくて細い
        
        layer.object3d = std::make_unique<Object3d>();
        layer.object3d->Initialize(dxCommon_, spriteCommon_);
        layer.object3d->SetEnableLighting(false);
        
        layer.radius = 4.0f;
        layer.innerRatio = 0.9f;
        layer.rotationSpeed = 0.3f;
        layer.currentRotation = 0.0f;
        layer.color = {1.0f, 0.8f, 0.2f, 0.8f}; // 金色
        layer.pulsePhase = 0.0f;
        layer.pulseSpeed = 1.5f;
        layer.divisions = 64;
        layer.reverse = false;
        
        layers_.push_back(std::move(layer));
    }
    
    // レイヤー2: 中間の魔法円（中速回転・逆方向）
    {
        MagicLayer layer;
        layer.ring = std::make_unique<Ring>();
        layer.ring->Initialize(dxCommon_);
        layer.ring->Generate(3.0f, 2.4f, 48);
        
        layer.object3d = std::make_unique<Object3d>();
        layer.object3d->Initialize(dxCommon_, spriteCommon_);
        layer.object3d->SetEnableLighting(false);
        
        layer.radius = 3.0f;
        layer.innerRatio = 0.8f;
        layer.rotationSpeed = -0.8f; // 逆回転
        layer.currentRotation = 0.0f;
        layer.color = {0.2f, 0.8f, 1.0f, 0.9f}; // 青色
        layer.pulsePhase = PI / 3.0f;
        layer.pulseSpeed = 2.0f;
        layer.divisions = 48;
        layer.reverse = true;
        
        layers_.push_back(std::move(layer));
    }
    
    // レイヤー3: 内側の魔法円（高速回転）
    {
        MagicLayer layer;
        layer.ring = std::make_unique<Ring>();
        layer.ring->Initialize(dxCommon_);
        layer.ring->Generate(2.0f, 1.2f, 32);
        
        layer.object3d = std::make_unique<Object3d>();
        layer.object3d->Initialize(dxCommon_, spriteCommon_);
        layer.object3d->SetEnableLighting(false);
        
        layer.radius = 2.0f;
        layer.innerRatio = 0.6f;
        layer.rotationSpeed = 1.5f;
        layer.currentRotation = 0.0f;
        layer.color = {1.0f, 0.2f, 0.8f, 1.0f}; // マゼンタ
        layer.pulsePhase = PI * 2.0f / 3.0f;
        layer.pulseSpeed = 3.0f;
        layer.divisions = 32;
        layer.reverse = false;
        
        layers_.push_back(std::move(layer));
    }
    
    // レイヤー4: ルーン文字的な細い円（超高速回転・逆方向）
    {
        MagicLayer layer;
        layer.ring = std::make_unique<Ring>();
        layer.ring->Initialize(dxCommon_);
        layer.ring->Generate(1.5f, 1.3f, 24); // 非常に細い
        
        layer.object3d = std::make_unique<Object3d>();
        layer.object3d->Initialize(dxCommon_, spriteCommon_);
        layer.object3d->SetEnableLighting(false);
        
        layer.radius = 1.5f;
        layer.innerRatio = 0.87f;
        layer.rotationSpeed = -2.5f; // 超高速逆回転
        layer.currentRotation = 0.0f;
        layer.color = {1.0f, 1.0f, 1.0f, 1.0f}; // 白色（ルーン文字）
        layer.pulsePhase = PI;
        layer.pulseSpeed = 4.0f;
        layer.divisions = 24;
        layer.reverse = true;
        
        layers_.push_back(std::move(layer));
    }
    
    // レイヤー5: 中心のコア（脈動のみ）
    {
        MagicLayer layer;
        layer.ring = std::make_unique<Ring>();
        layer.ring->Initialize(dxCommon_);
        layer.ring->Generate(0.8f, 0.2f, 16);
        
        layer.object3d = std::make_unique<Object3d>();
        layer.object3d->Initialize(dxCommon_, spriteCommon_);
        layer.object3d->SetEnableLighting(false);
        
        layer.radius = 0.8f;
        layer.innerRatio = 0.25f;
        layer.rotationSpeed = 0.1f; // ゆっくり
        layer.currentRotation = 0.0f;
        layer.color = {1.0f, 0.5f, 0.0f, 1.0f}; // オレンジ色（エネルギーコア）
        layer.pulsePhase = PI * 4.0f / 3.0f;
        layer.pulseSpeed = 6.0f;
        layer.divisions = 16;
        layer.reverse = false;
        
        layers_.push_back(std::move(layer));
    }
}

void MagicCircleEffect::Update(float deltaTime) {
    if (!isActive_ || !isVisible_) return;
    
    effectTime_ += deltaTime;
    UpdateLayers(deltaTime);
}

void MagicCircleEffect::UpdateLayers(float deltaTime) {
    for (auto& layer : layers_) {
        // 回転更新
        layer.currentRotation += layer.rotationSpeed * deltaTime;
        
        // 脈動効果
        float pulseValue = 1.0f + PULSE_AMPLITUDE * std::sinf(effectTime_ * layer.pulseSpeed + layer.pulsePhase);
        
        // カラーの輝度調整（脈動）
        Vector4 currentColor = layer.color;
        currentColor.x *= pulseValue * intensity_;
        currentColor.y *= pulseValue * intensity_;
        currentColor.z *= pulseValue * intensity_;
        currentColor.w *= std::min<float>(1.0f, pulseValue * intensity_);
        
        // Object3Dの更新
        if (layer.object3d) {
            layer.object3d->SetPosition(position_);
            layer.object3d->SetRotation({0.0f, 0.0f, layer.currentRotation});
            layer.object3d->SetScale({scale_ * pulseValue, scale_ * pulseValue, scale_});
            layer.object3d->SetColor(currentColor);
            
            // UVスクロール（魔法的な流れ効果）
            float uvScrollU = effectTime_ * layer.rotationSpeed * 0.1f;
            float uvScrollV = effectTime_ * layer.pulseSpeed * 0.05f;
            layer.object3d->SetUVScroll(uvScrollU, uvScrollV);
            
            if (camera_) {
                layer.object3d->SetCamera(camera_);
                layer.object3d->Update();
            }
        }
    }
}

void MagicCircleEffect::Draw() {
    if (!isVisible_ || !isActive_) return;
    
    // 外側から内側の順に描画（奥行き感）
    for (auto& layer : layers_) {
        if (layer.ring && layer.object3d) {
            assert(dxCommon_);
            
            // 共通描画設定
            spriteCommon_->CommonDraw();
            
            // Ringの頂点バッファをセット
            dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &layer.ring->GetVBView());
            
            // マテリアルCBufferの場所を設定
            dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, 
                layer.object3d->GetMaterialResource()->GetGPUVirtualAddress());
            
            // 変換行列CBufferの場所を設定
            dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, 
                layer.object3d->GetTransformationMatrixResource()->GetGPUVirtualAddress());
            
            // テクスチャの場所を設定
            const std::string texturePath = "Resources/particle/gradationLine.png";
            dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2,
                TextureManager::GetInstance()->GetSrvHandleGPU(texturePath));
            
            // ライトCBufferの場所を設定
            dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, 
                layer.object3d->GetDirectionalLightResource()->GetGPUVirtualAddress());
            
            // 描画
            dxCommon_->GetCommandList()->DrawInstanced(layer.ring->GetVertexCount(), 1, 0, 0);
        }
    }
}

void MagicCircleEffect::SetCamera(Camera* camera) {
    camera_ = camera;
}

void MagicCircleEffect::SetPosition(const Vector3& position) {
    position_ = position;
}

void MagicCircleEffect::SetScale(float scale) {
    scale_ = scale;
}

void MagicCircleEffect::SetIntensity(float intensity) {
    intensity_ = std::clamp(intensity, 0.0f, 2.0f);
}

void MagicCircleEffect::StartEffect() {
    isActive_ = true;
    effectTime_ = 0.0f;
    OutputDebugStringA("MagicCircleEffect::StartEffect - Epic magic circle activated!\n");
}

void MagicCircleEffect::StopEffect() {
    isActive_ = false;
    OutputDebugStringA("MagicCircleEffect::StopEffect - Magic circle deactivated\n");
}
