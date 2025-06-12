#include "EffekseerManager.h"
#include <cassert>

// Note: Effekseer headers will be included when the library is properly built
// Currently using placeholder implementation until Effekseer library is built

EffekseerManager::~EffekseerManager() {
    ForceReleaseResources();
}

void EffekseerManager::ForceReleaseResources() {
    OutputDebugStringA("EffekseerManager::ForceReleaseResources - Starting resource cleanup\n");

    // アクティブエフェクトの停止とクリア
    StopAllEffects();
    activeEffects_.clear();
    OutputDebugStringA("EffekseerManager: All active effects cleared\n");

    // エフェクトリソースのクリア
    effects_.clear();
    OutputDebugStringA("EffekseerManager: All effect resources cleared\n");

    // Effekseerリソースの解放
    if (efkRenderer_) {
        efkRenderer_.reset();
        OutputDebugStringA("EffekseerManager: Renderer released\n");
    }

    if (efkManager_) {
        efkManager_.reset();
        OutputDebugStringA("EffekseerManager: Manager released\n");
    }

    isInitialized_ = false;
    OutputDebugStringA("EffekseerManager::ForceReleaseResources - Resource cleanup completed\n");
}

bool EffekseerManager::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;

    OutputDebugStringA("EffekseerManager::Initialize - Starting initialization\n");

    // 現在は暫定実装：簡易エフェクトシステムとしてParticleManagerを使用
    // 実際のEffekseerライブラリがリンクされるまでの代替手段
    
    OutputDebugStringA("EffekseerManager: Using ParticleManager as fallback for effects\n");
    OutputDebugStringA("EffekseerManager: To enable full Effekseer rendering, follow EFFEKSEER_SETUP.md\n");
    
    isInitialized_ = true;
    return true;
}

void EffekseerManager::Update(const Camera* camera) {
    if (!isInitialized_ || !efkManager_) {
        return;
    }

    // TODO: Implement Effekseer manager update
    // This would include:
    // 1. Set layer parameters (viewer position, etc.)
    // 2. Call efkManager->Update() with update parameters
}

void EffekseerManager::Draw(const Camera* camera) {
    if (!isInitialized_ || !efkRenderer_ || !efkManager_) {
        return;
    }

    // TODO: Implement Effekseer rendering
    // This would include:
    // 1. Set time for renderer
    // 2. Set camera and projection matrices
    // 3. Begin rendering
    // 4. Draw effects with draw parameters
    // 5. End rendering
}

bool EffekseerManager::LoadEffect(const std::string& name, const std::string& filePath) {
    if (!isInitialized_) {
        OutputDebugStringA(("EffekseerManager::LoadEffect - Manager not initialized: " + name + "\n").c_str());
        return false;
    }

    if (useParticleFallback_) {
        // ParticleManagerを使用したフォールバック実装
        std::string particleTexture;
        
        if (name.find("laser") != std::string::npos) {
            // レーザーエフェクト用のパーティクルテクスチャ
            particleTexture = "Resources/particle/laser.png";
            ParticleManager::GetInstance()->CreateParticleGroup(name + "_particles", particleTexture);
        } else if (name.find("explosion") != std::string::npos) {
            // 爆発エフェクト用のパーティクルテクスチャ
            particleTexture = "Resources/particle/explosion.png";
            ParticleManager::GetInstance()->CreateParticleGroup(name + "_particles", particleTexture);
        } else {
            // デフォルトエフェクト
            particleTexture = "Resources/particle/smoke.png";
            ParticleManager::GetInstance()->CreateParticleGroup(name + "_particles", particleTexture);
        }
        
        OutputDebugStringA(("EffekseerManager::LoadEffect - Fallback using particles: " + name + "\n").c_str());
        return true;
    }

    // TODO: Implement actual Effekseer effect loading when library is linked
    OutputDebugStringA(("EffekseerManager::LoadEffect - Placeholder load: " + name + " from " + filePath + "\n").c_str());
    return true;
}

::Effekseer::Handle EffekseerManager::PlayEffect(const std::string& name, const Vector3& position) {
    if (!isInitialized_) {
        return -1;
    }

    if (useParticleFallback_) {
        // ParticleManagerを使用したフォールバック実装で視覚的エフェクトを作成
        std::string particleName = name + "_particles";
        
        if (name.find("laser") != std::string::npos) {
            // レーザーエフェクト：直線状に高速パーティクル（120fps最適化）
            ParticleManager::GetInstance()->Emit(
                particleName, position, 20, // 120fps確実実現のため軽量化
                Vector3{-4.0f, -1.0f, -1.0f}, Vector3{4.0f, 1.0f, 1.0f}, // 120fps用速度調整
                Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, 0.0f}, // 加速度
                0.1f, 0.3f, // 開始サイズ
                0.0f, 0.1f, // 終了サイズ  
                Vector4{1.0f, 0.2f, 0.2f, 1.0f}, Vector4{1.0f, 0.8f, 0.8f, 1.0f}, // 開始色（赤系）
                Vector4{1.0f, 0.0f, 0.0f, 0.0f}, Vector4{1.0f, 0.2f, 0.2f, 0.0f}, // 終了色
                0.0f, 360.0f, // 回転
                -360.0f, 360.0f, // 120fps用回転速度調整
                0.6f, 1.2f // 120fps用寿命調整
            );
        } else if (name.find("explosion") != std::string::npos) {
            // 爆発エフェクト：球状に拡散するパーティクル（120fps最適化）
            ParticleManager::GetInstance()->Emit(
                particleName, position, 50, // 120fps確実実現のため軽量化
                Vector3{-6.0f, -6.0f, -6.0f}, Vector3{6.0f, 6.0f, 6.0f}, // 120fps用速度調整
                Vector3{0.0f, -4.0f, 0.0f}, Vector3{0.0f, -2.0f, 0.0f}, // 120fps用重力調整
                0.5f, 1.5f, // 開始サイズ
                0.0f, 0.2f, // 終了サイズ
                Vector4{1.0f, 1.0f, 0.2f, 1.0f}, Vector4{1.0f, 0.8f, 0.0f, 1.0f}, // 開始色（オレンジ系）
                Vector4{1.0f, 0.2f, 0.0f, 0.0f}, Vector4{0.8f, 0.0f, 0.0f, 0.0f}, // 終了色（赤系）
                0.0f, 360.0f, // 回転
                -180.0f, 180.0f, // 120fps用回転速度調整
                1.2f, 2.5f // 120fps用寿命調整
            );
        } else {
            // デフォルトエフェクト
            ParticleManager::GetInstance()->Emit(particleName, position, 30);
        }
        
        int handle = nextHandle_++;
        activeEffects_[name] = handle;
        
        OutputDebugStringA(("EffekseerManager::PlayEffect - Fallback particles played: " + name + " at (" + 
                           std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")\n").c_str());
        return handle;
    }

    // TODO: Implement actual Effekseer effect playback when library is linked
    OutputDebugStringA(("EffekseerManager::PlayEffect - Placeholder play: " + name + "\n").c_str());
    return 0; // Placeholder handle
}

void EffekseerManager::StopEffect(::Effekseer::Handle handle) {
    if (!isInitialized_ || !efkManager_ || handle < 0) {
        return;
    }

    // TODO: Implement effect stop
    // This would include:
    // 1. Call efkManager->StopEffect(handle)
    // 2. Remove handle from activeEffects_ map
}

void EffekseerManager::StopAllEffects() {
    if (!isInitialized_ || !efkManager_) {
        return;
    }

    // TODO: Implement stop all effects
    // This would include:
    // 1. Call efkManager->StopAllEffects()
    // 2. Clear activeEffects_ map
    
    activeEffects_.clear();
    OutputDebugStringA("EffekseerManager::StopAllEffects - All effects stopped\n");
}

void EffekseerManager::SetEffectPosition(::Effekseer::Handle handle, const Vector3& position) {
    if (!isInitialized_ || !efkManager_ || handle < 0) {
        return;
    }

    // TODO: Implement position setting
    // This would include:
    // 1. Call efkManager->SetLocation() with handle and position
}

void EffekseerManager::SetEffectRotation(::Effekseer::Handle handle, const Vector3& rotation) {
    if (!isInitialized_ || !efkManager_ || handle < 0) {
        return;
    }

    // TODO: Implement rotation setting
    // This would include:
    // 1. Call efkManager->SetRotation() with handle and rotation
}

void EffekseerManager::SetEffectScale(::Effekseer::Handle handle, const Vector3& scale) {
    if (!isInitialized_ || !efkManager_ || handle < 0) {
        return;
    }

    // TODO: Implement scale setting
    // This would include:
    // 1. Call efkManager->SetScale() with handle and scale
}

bool EffekseerManager::IsPlaying(::Effekseer::Handle handle) {
    if (!isInitialized_ || !efkManager_ || handle < 0) {
        return false;
    }

    // TODO: Implement playing check
    // This would include:
    // 1. Call efkManager->Exists(handle) to check if effect is still playing
    
    return false; // Placeholder
}

int EffekseerManager::GetActiveEffectCount() const {
    return static_cast<int>(activeEffects_.size());
}