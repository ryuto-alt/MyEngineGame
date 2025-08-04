#pragma once
#include <chrono>
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
#include "Skybox.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "Particle3DManager.h"
#include "Particle3DEmitter.h"
#include "HitEffect3D.h"
#include "EffectManager3D.h"

// アニメーション関連
#include "AnimatedModel.h"
#include "Animation.h"
#include "AnimationPlayer.h"
#include "AnimationBlender.h"
#include "AnimationUtility.h"

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

// 前方宣言
class OffscreenRenderingManager;
enum class ProcessingMode;

// UnoEngineクラス - DirectX12ゲームエンジン統合クラス
class UnoEngine {
public:
    // シングルトンインスタンスを取得
    static UnoEngine* GetInstance();
    
    // シングルトンインスタンスを破棄
    static void DestroyInstance();

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
    // キーボード入力
    bool IsKeyPressed(int key) const { return input_->PushKey(key); }
    bool IsKeyTriggered(int key) const { return input_->TriggerKey(key); }
    
    // 統合移動入力（キーボード＋ゲームパッド）
    Vector2 GetMovementInput(float deadZone = 0.1f) const;
    bool IsMoving() const;
    
    // 簡易キー入力（よく使うキー）
    bool IsMoveForward() const { return IsKeyPressed(DIK_W); }
    bool IsMoveBackward() const { return IsKeyPressed(DIK_S); }
    bool IsMoveLeft() const { return IsKeyPressed(DIK_A); }
    bool IsMoveRight() const { return IsKeyPressed(DIK_D); }
    bool IsJump() const { return IsKeyPressed(DIK_SPACE); }
    bool IsSprint() const { return IsKeyPressed(DIK_LSHIFT); }
    bool IsInteract() const { return IsKeyTriggered(DIK_F); }
    bool IsPause() const { return IsKeyTriggered(DIK_ESCAPE); }
    
    // マウス入力
    void GetMouseMovement(float& deltaX, float& deltaY) { input_->GetMouseMovement(deltaX, deltaY); }
    void SetMouseCursor(bool visible) { input_->SetMouseCursor(visible); }
    void ResetMouseCenter() { input_->ResetMouseCenter(); }
    
    // === Xboxコントローラー入力システム ===
    bool IsGamepadConnected(int playerIndex = 0) const { return input_->IsXboxControllerConnected(playerIndex); }
    bool IsGamepadButtonPressed(int button, int playerIndex = 0) const { return input_->IsXboxButtonPressed(button, playerIndex); }
    bool IsGamepadButtonTriggered(int button, int playerIndex = 0) const { return input_->IsXboxButtonTriggered(button, playerIndex); }
    
    // スティック入力（デッドゾーン処理済み）
    Vector2 GetLeftStick(float deadZone = 0.1f, int playerIndex = 0) const;
    Vector2 GetRightStick(float deadZone = 0.1f, int playerIndex = 0) const;
    
    // トリガー入力
    float GetLeftTrigger(int playerIndex = 0) const { return input_->GetXboxLeftTrigger(playerIndex); }
    float GetRightTrigger(int playerIndex = 0) const { return input_->GetXboxRightTrigger(playerIndex); }
    
    // === カメラシステム ===
    // 基本カメラ操作
    void SetCameraPosition(const Vector3& position) { camera_->SetTranslate(position); }
    void SetCameraRotation(const Vector3& rotation) { camera_->SetRotate(rotation); }
    Vector3 GetCameraPosition() const { return camera_->GetTranslate(); }
    Vector3 GetCameraRotation() const { return camera_->GetRotate(); }
    void SetCameraFov(float fov) { camera_->SetFovY(fov); }
    
    // カメラの方向ベクトル
    Vector3 GetCameraForward() const { return camera_->GetForwardVector(); }
    Vector3 GetCameraRight() const { return camera_->GetRightVector(); }
    Vector3 GetCameraUp() const { return camera_->GetUpVector(); }
    
    // カメラ入力処理（統合版）
    void UpdateCameraInput();  // マウス＋右スティック入力を自動処理
    
    // カメラ感度設定
    void SetMouseSensitivity(float sensitivity) { camera_->SetMouseSensitivity(sensitivity); }
    void SetStickSensitivity(float sensitivity) { camera_->SetStickSensitivity(sensitivity); }
    
    // オービットカメラ（追従カメラ）
    void SetCameraTarget(const Vector3& target) { camera_->SetOrbitTarget(target); }
    void SetCameraDistance(float distance) { camera_->SetOrbitDistance(distance); }
    void SetCameraHeight(float height) { camera_->SetOrbitHeight(height); }
    
    // フリーカメラモード
    void UpdateFreeCameraMovement();  // WASDキーでカメラを直接移動
#ifdef _DEBUG
    bool IsFreeCameraMode() const { return camera_->IsFreeCameraMode(); }
    void ToggleFreeCameraMode() { camera_->ToggleFreeCameraMode(); }
#else
    bool IsFreeCameraMode() const { return false; }
    void ToggleFreeCameraMode() {}
#endif
    
    // 旧API（互換性のため残す）
    void UpdateCameraMouse();
    void UpdateCameraRightStick();
    
    // === オーディオシステム ===
    bool LoadAudio(const std::string& name, const std::string& filePath);
    void PlayAudio(const std::string& name, bool loop = false, float volume = 1.0f);
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
    
    // === オフスクリーンレンダリング ===
    void EnablePostProcessing(bool enable = true);
    void SetPostProcessMode(int mode);  // 0:Normal, 1:Grayscale, 2:Vignetting
    void BeginOffscreenRendering();
    void EndOffscreenRendering();
    
    // === アニメーションシステム ===
    std::unique_ptr<AnimatedModel> CreateAnimatedModel();
    Animation LoadAnimation(const std::string& directoryPath, const std::string& filename);
    
    // === 2Dスプライト作成システム ===
    std::unique_ptr<Sprite> CreateSprite(const std::string& texturePath);
    
    // === 簡榁化API ===
    void LoadTexture(const std::string& path);
    std::unique_ptr<Skybox> CreateSkybox();
    void LoadSkybox(Skybox* skybox, const std::string& path);
    
    template<typename T>
    void SetEnvMap(T* obj) {
        Skybox::SetEnvMap(obj);
    }
    
    template<typename T>
    void SetEnvMap(std::unique_ptr<T>& obj) {
        Skybox::SetEnvMap(obj);
    }
    
    // === 衝突判定システム ===
    bool CheckCollision(const Vector3& pos1, float radius1, const Vector3& pos2, float radius2);
    
    // === スムージングシステム ===
    // 角度を-π～πの範囲に正規化
    float NormalizeAngle(float angle);
    // 角度の最短距離を計算
    float AngleDifference(float from, float to);
    // 角度を線形補間
    float LerpAngle(float from, float to, float t);
    // 値を線形補間
    float Lerp(float from, float to, float t);
    // Vector3を線形補間
    Vector3 LerpVector3(const Vector3& from, const Vector3& to, float t);
    // 回転角度をスムージング（deltaTime考慮）
    float SmoothRotation(float current, float target, float speed, float deltaTime);
    
    // === 時間管理 ===
    // デルタタイムを取得（秒単位）
    float GetDeltaTime() const { return deltaTime_; }
    // 前フレームからの経過時間を更新（フレーム開始時に呼ぶ）
    void UpdateDeltaTime();
    
    // === シーン管理 ===
    void ChangeScene(const std::string& sceneName);
    
    // === デバッグ情報 ===
    void ShowDebugInfo();
    
    // === 従来のアクセサ ===
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
    
    // オフスクリーンレンダリング
    class OffscreenRenderingManager* offscreenManager_ = nullptr;
    bool postProcessingEnabled_ = false;

    // コンストラクタ（シングルトン）
    UnoEngine() = default;
    // デストラクタ（シングルトン）
    ~UnoEngine() = default;
    
    // 終了処理済みフラグ
    bool finalized_ = false;

    // 基本コンポーネント
    std::unique_ptr<WinApp> winApp_;
    std::unique_ptr<DirectXCommon> dxCommon_;
    std::unique_ptr<Input> input_;
    std::unique_ptr<Camera> camera_;

    // グラフィックス関連コンポーネント
    std::unique_ptr<SpriteCommon> spriteCommon_;
    std::unique_ptr<SrvManager> srvManager_;

    // シーンファクトリー 
    std::unique_ptr<SceneFactory> sceneFactory_;
    
    // 3D空間オーディオ関連
    std::unique_ptr<SpatialAudioListener> audioListener_;
    std::vector<std::unique_ptr<SpatialAudioSource>> spatialAudioSources_;

    // ImGuiの初期化
    void InitializeImGui();

    // 終了リクエストフラグ
    bool endRequest_ = false;
    
    // 時間管理
    float deltaTime_ = 0.0f;  // 前フレームからの経過時間（秒）
    std::chrono::steady_clock::time_point lastFrameTime_;  // 前フレームの時刻
};