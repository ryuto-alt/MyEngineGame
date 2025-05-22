// 簡易テスト用CircleEffect - 最小限の実装
// src/Engine/Graphics/CircleEffect.cpp
#include "CircleEffect.h"
#include "TextureManager.h"
#include <cassert>

CircleEffect::CircleEffect()
    : dxCommon_(nullptr)
    , spriteCommon_(nullptr)
    , isUVAnimating_(false)
    , uvScrollSpeedU_(0.0f)
    , uvScrollSpeedV_(0.0f)
    , currentUVScrollU_(0.0f)
    , currentUVScrollV_(0.0f)
    , isVisible_(true)
    , effectTime_(0.0f) {
}

CircleEffect::~CircleEffect() {}

void CircleEffect::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    assert(dxCommon);
    assert(spriteCommon);
    
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    
    // Object3dの初期化
    object3d_ = std::make_unique<Object3d>();
    object3d_->Initialize(dxCommon_, spriteCommon_);
    
    OutputDebugStringA("CircleEffect::Initialize - Initialized successfully\n");
}

void CircleEffect::CreateCircleEffect(float outerRadius, float innerRadius, uint32_t divisions) {
    // Ringプリミティブの作成
    ring_ = std::make_unique<Ring>();
    ring_->Initialize(dxCommon_);
    ring_->Generate(outerRadius, innerRadius, divisions);
    
    // gradationLine.pngテクスチャの設定
    const std::string texturePath = "Resources/particle/gradationLine.png";
    
    // テクスチャの事前読み込み
    if (!TextureManager::GetInstance()->IsTextureExists(texturePath)) {
        TextureManager::GetInstance()->LoadTexture(texturePath);
    }
    
    // CircleEffectでは直接Ringを描画するためModelは使用しない
    
    // エフェクト用の初期設定
    object3d_->SetEnableLighting(false);
    object3d_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    
    OutputDebugStringA("CircleEffect::CreateCircleEffect - Created successfully\n");
}

void CircleEffect::Update() {
    if (!isVisible_) return;
    
    effectTime_ += 1.0f / 60.0f;
    
    // UVアニメーションの更新
    if (isUVAnimating_) {
        currentUVScrollU_ += uvScrollSpeedU_;
        currentUVScrollV_ += uvScrollSpeedV_;
        
        if (object3d_) {
            object3d_->SetUVScroll(currentUVScrollU_, currentUVScrollV_);
        }
    }
    
    // Object3dの更新
    if (object3d_) {
        object3d_->Update();
    }
}

void CircleEffect::Draw() {
    if (!isVisible_) return;
    
    // Ringがある場合はRingを直接描画
    if (ring_ && object3d_) {
        assert(dxCommon_);
        
        // 共通描画設定
        spriteCommon_->CommonDraw();
        
        // Ringの頂点バッファをセット
        dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &ring_->GetVBView());
        
        // マテリアルCBufferの場所を設定
        dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, object3d_->GetMaterialResource()->GetGPUVirtualAddress());
        
        // 変換行列CBufferの場所を設定
        dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, object3d_->GetTransformationMatrixResource()->GetGPUVirtualAddress());
        
        // テクスチャの場所を設定
        const std::string texturePath = "Resources/particle/gradationLine.png";
        dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2,
            TextureManager::GetInstance()->GetSrvHandleGPU(texturePath));
        
        // ライトCBufferの場所を設定
        dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, object3d_->GetDirectionalLightResource()->GetGPUVirtualAddress());
        
        // 描画
        dxCommon_->GetCommandList()->DrawInstanced(ring_->GetVertexCount(), 1, 0, 0);
    }
}

void CircleEffect::SetCamera(Camera* camera) {
    if (object3d_) {
        object3d_->SetCamera(camera);
    }
}

void CircleEffect::SetPosition(const Vector3& position) {
    if (object3d_) {
        object3d_->SetPosition(position);
    }
}

void CircleEffect::SetRotation(const Vector3& rotation) {
    if (object3d_) {
        object3d_->SetRotation(rotation);
    }
}

void CircleEffect::SetScale(const Vector3& scale) {
    if (object3d_) {
        object3d_->SetScale(scale);
    }
}

void CircleEffect::SetColor(const Vector4& color) {
    if (object3d_) {
        object3d_->SetColor(color);
    }
}

void CircleEffect::SetUVScroll(float scrollU, float scrollV) {
    currentUVScrollU_ = scrollU;
    currentUVScrollV_ = scrollV;
    
    if (object3d_) {
        object3d_->SetUVScroll(scrollU, scrollV);
    }
}

void CircleEffect::StartUVAnimation(float speedU, float speedV) {
    isUVAnimating_ = true;
    uvScrollSpeedU_ = speedU;
    uvScrollSpeedV_ = speedV;
    
    OutputDebugStringA("CircleEffect::StartUVAnimation - Started\n");
}

void CircleEffect::StopUVAnimation() {
    isUVAnimating_ = false;
    uvScrollSpeedU_ = 0.0f;
    uvScrollSpeedV_ = 0.0f;
    
    OutputDebugStringA("CircleEffect::StopUVAnimation - Stopped\n");
}

const Vector3& CircleEffect::GetPosition() const {
    static Vector3 defaultPos = {0.0f, 0.0f, 0.0f};
    return object3d_ ? object3d_->GetPosition() : defaultPos;
}

const Vector3& CircleEffect::GetRotation() const {
    static Vector3 defaultRot = {0.0f, 0.0f, 0.0f};
    return object3d_ ? object3d_->GetRotation() : defaultRot;
}

const Vector3& CircleEffect::GetScale() const {
    static Vector3 defaultScale = {1.0f, 1.0f, 1.0f};
    return object3d_ ? object3d_->GetScale() : defaultScale;
}
