// src/Engine/Graphics/RingWrapper.cpp
#include "RingWrapper.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include <cassert>

RingWrapper::RingWrapper() 
    : ring_(nullptr)
    , advancedRing_(nullptr)
    , dxCommon_(nullptr)
    , hasValidRing_(false) {
}

RingWrapper::~RingWrapper() {}

void RingWrapper::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    assert(dxCommon);
    assert(spriteCommon);
    
    dxCommon_ = dxCommon;
    
    // Object3dの初期化
    object3d_ = std::make_unique<Object3d>();
    object3d_->Initialize(dxCommon, spriteCommon);
    
    // ダミーモデルの作成
    CreateDummyModel();
    
    // デフォルト設定
    object3d_->SetPosition({0.0f, 0.0f, 0.0f});
    object3d_->SetRotation({0.0f, 0.0f, 0.0f});
    object3d_->SetScale({1.0f, 1.0f, 1.0f});
    object3d_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    object3d_->SetEnableLighting(false); // Ringエフェクトではライティング無効
}

void RingWrapper::SetRing(Ring* ring) {
    ring_ = ring;
    advancedRing_ = nullptr;
    hasValidRing_ = (ring_ != nullptr);
    
    if (hasValidRing_) {
        // Ringが設定された場合、ダミーモデルを再作成
        CreateDummyModel();
    }
}

void RingWrapper::SetAdvancedRing(AdvancedRing* ring) {
    advancedRing_ = ring;
    ring_ = nullptr;
    hasValidRing_ = (advancedRing_ != nullptr);
    
    if (hasValidRing_) {
        // AdvancedRingが設定された場合、ダミーモデルを再作成
        CreateDummyModel();
    }
}

void RingWrapper::Update() {
    if (object3d_) {
        object3d_->Update();
    }
}

void RingWrapper::Draw() {
    if (object3d_ && hasValidRing_) {
        // カスタム描画処理
        assert(dxCommon_);
        
        if (ring_) {
            // 基本Ringの描画
            dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &ring_->GetVBView());
            
            // Material, Transform, Lightの設定（Object3dから借用）
            // 実際の実装では専用のシェーダーとパイプラインが必要
            
            // 描画コマンド
            dxCommon_->GetCommandList()->DrawInstanced(ring_->GetVertexCount(), 1, 0, 0);
            
        } else if (advancedRing_) {
            // 拡張Ringの描画
            dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &advancedRing_->GetVBView());
            
            // 描画コマンド
            dxCommon_->GetCommandList()->DrawInstanced(advancedRing_->GetVertexCount(), 1, 0, 0);
        }
    }
}

void RingWrapper::SetCamera(Camera* camera) {
    if (object3d_) {
        object3d_->SetCamera(camera);
    }
}

void RingWrapper::SetPosition(const Vector3& position) {
    if (object3d_) {
        object3d_->SetPosition(position);
    }
}

void RingWrapper::SetRotation(const Vector3& rotation) {
    if (object3d_) {
        object3d_->SetRotation(rotation);
    }
}

void RingWrapper::SetScale(const Vector3& scale) {
    if (object3d_) {
        object3d_->SetScale(scale);
    }
}

void RingWrapper::SetColor(const Vector4& color) {
    if (object3d_) {
        object3d_->SetColor(color);
    }
}

const Vector3& RingWrapper::GetPosition() const {
    static Vector3 defaultPos = {0.0f, 0.0f, 0.0f};
    return object3d_ ? object3d_->GetPosition() : defaultPos;
}

const Vector3& RingWrapper::GetRotation() const {
    static Vector3 defaultRot = {0.0f, 0.0f, 0.0f};
    return object3d_ ? object3d_->GetRotation() : defaultRot;
}

const Vector3& RingWrapper::GetScale() const {
    static Vector3 defaultScale = {1.0f, 1.0f, 1.0f};
    return object3d_ ? object3d_->GetScale() : defaultScale;
}

void RingWrapper::CreateDummyModel() {
    // 簡易ダミーモデルの作成
    // 実際の実装では、RingプリミティブをModelクラスに統合するか、
    // Ring専用の描画パイプラインを作成する必要があります
    
    // ここでは基本的なテクスチャ設定のみ行う
    dummyModel_ = std::make_unique<Model>();
    dummyModel_->Initialize(dxCommon_);
    
    // デフォルトテクスチャの読み込み
    TextureManager::GetInstance()->LoadDefaultTexture();
    
    if (object3d_) {
        object3d_->SetModel(dummyModel_.get());
    }
}