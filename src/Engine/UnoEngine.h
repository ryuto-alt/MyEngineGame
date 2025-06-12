#pragma once
// 基本システム
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Camera.h"

// グラフィックス関連
#include "SpriteCommon.h"
#include "Sprite.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "Object3d.h"
#include "Model.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "Particle3DManager.h"
#include "Particle3DEmitter.h"
#include "HitEffect3D.h"
#include "EffectManager3D.h"

// 衝突判定関連
#include "Collision.h"
#include "CollisionPrimitive.h"
#include "CollisionUtility.h"
#include "CollisionManager.h"

// オーディオ関連
#include "AudioManager.h"
#include "SpatialAudioSource.h"
#include "SpatialAudioListener.h"

// シーン管理
#include "SceneManager.h"
#include "SceneFactory.h" // 抽象クラスのみをインクルード
#include "IScene.h"

// 数学・ユーティリティ関連
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Mymath.h"
#include "Logger.h"
#include "StringUtility.h"

// ImGui関連
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <memory>
#include <string>
#include <vector>

// UnoEngineクラス - DirectX12ゲームエンジン統合クラス
class UnoEngine {
public:
    // シングルトンインスタンスを取得
    static UnoEngine* GetInstance();

    // コピー禁止
    UnoEngine(const UnoEngine&) = delete;
    UnoEngine& operator=(const UnoEngine&) = delete;

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

    // 終了処理
    void Finalize();

    // ゲームループ実行
    void Run();

    // 終了リクエスト
    bool IsEndRequested() const { return endRequest_; }
    void RequestEnd() { endRequest_ = true; }

    // === 統一された直感的API ===
    
    // === 入力システム ===
    bool IsKeyPressed(int key) const { return input_->PushKey(key); }
    bool IsKeyTriggered(int key) const { return input_->TriggerKey(key); }
    
    // === カメラシステム ===
    void SetCameraPosition(const Vector3& position) { camera_->SetTranslate(position); }
    void SetCameraRotation(const Vector3& rotation) { camera_->SetRotate(rotation); }
    Vector3 GetCameraPosition() const { return camera_->GetTranslate(); }
    
    // === オーディオシステム ===
    bool LoadAudio(const std::string& name, const std::string& filePath);
    void PlayAudio(const std::string& name, bool loop = false);
    void StopAudio(const std::string& name);
    void SetAudioVolume(const std::string& name, float volume);
    bool IsAudioPlaying(const std::string& name);
    
    // === 3D空間オーディオシステム ===
    std::unique_ptr<SpatialAudioSource> CreateSpatialAudioSource(const std::string& audioName, const Vector3& position);
    void SetAudioListenerPosition(const Vector3& position);
    void SetAudioListenerOrientation(const Vector3& forward, const Vector3& up = Vector3{0.0f, 1.0f, 0.0f});
    void UpdateSpatialAudio();  // 毎フレーム呼び出して3Dオーディオを更新
    
    // === パーティクルシステム ===
    bool CreateParticleEffect(const std::string& name, const std::string& texturePath);
    void PlayParticle(const std::string& name, const Vector3& position, int count = 10);
    void PlayParticle(const std::string& name, const Vector3& position, int count,
                     const Vector3& velocity, float lifeTime = 3.0f);
    
    // === 3Dオブジェクト作成システム ===
    std::unique_ptr<Object3d> CreateObject3D();
    std::unique_ptr<Model> LoadModel(const std::string& modelPath);
    
    // === 2Dスプライト作成システム ===
    std::unique_ptr<Sprite> CreateSprite(const std::string& texturePath);
    
    // === テクスチャ読み込み ===
    uint32_t LoadTexture(const std::string& filePath);
    
    // === 衝突判定システム ===
    bool CheckCollision(const Vector3& pos1, float radius1, const Vector3& pos2, float radius2);
    
    // === シーン管理 ===
    void ChangeScene(const std::string& sceneName);
    
    // === デバッグ情報 ===
    void ShowDebugInfo();
    
    // === 従来のアクセサ（上級者向け・必要時のみ使用） ===
    WinApp* GetWinApp() const { return winApp_.get(); }
    DirectXCommon* GetDirectXCommon() const { return dxCommon_.get(); }
    Input* GetInput() const { return input_.get(); }
    Camera* GetCamera() const { return camera_.get(); }
    SpriteCommon* GetSpriteCommon() const { return spriteCommon_.get(); }
    SrvManager* GetSrvManager() const { return srvManager_.get(); }
    SceneManager* GetSceneManager() const { return SceneManager::GetInstance(); }
    TextureManager* GetTextureManager() const { return TextureManager::GetInstance(); }
    ParticleManager* GetParticleManager() const { return ParticleManager::GetInstance(); }
    Particle3DManager* GetParticle3DManager() const { return Particle3DManager::GetInstance(); }
    EffectManager3D* GetEffectManager3D() const { return EffectManager3D::GetInstance(); }
    AudioManager* GetAudioManager() const { return AudioManager::GetInstance(); }
    Collision::CollisionManager* GetCollisionManager() const { return Collision::CollisionManager::GetInstance(); }

    // シーンファクトリーのセッター
    void SetSceneFactory(SceneFactory* sceneFactory);

private:
    // シングルトンインスタンス
    static UnoEngine* instance_;

    // コンストラクタ（シングルトン）
    UnoEngine() = default;
    // デストラクタ（シングルトン）
    ~UnoEngine() = default;

    // 基本コンポーネント
    std::unique_ptr<WinApp> winApp_;
    std::unique_ptr<DirectXCommon> dxCommon_;
    std::unique_ptr<Input> input_;
    std::unique_ptr<Camera> camera_;

    // グラフィックス関連コンポーネント
    std::unique_ptr<SpriteCommon> spriteCommon_;
    std::unique_ptr<SrvManager> srvManager_;

    // シーンファクトリー (SceneFactoryへのポインタとして保存)
    std::unique_ptr<SceneFactory> sceneFactory_;
    
    // 3D空間オーディオ関連
    std::unique_ptr<SpatialAudioListener> audioListener_;
    std::vector<std::unique_ptr<SpatialAudioSource>> spatialAudioSources_;

    // ImGuiの初期化
    void InitializeImGui();

    // 終了リクエストフラグ
    bool endRequest_ = false;
};