#pragma once
#include "UnoEngine.h"

class Player {
public:
    Player();
    ~Player();

    void Initialize(Camera* camera);
    void Update(UnoEngine* engine);
    void Draw();
    void Finalize();
    
    // 位置・回転の取得
    Vector3 GetPosition() const { return position_; }
    float GetRotationY() const { return currentRotationY_; }
    
    // ライトの設定
    void SetDirectionalLight(const DirectionalLight& light);
    void SetSpotLight(const SpotLight& light);
    
    // 環境マップテクスチャの設定
    void SetEnvironmentTexture(const std::string& texturePath);
    
    // アニメーション制御
    void PauseAnimation();
    void PlayAnimation();
    void ResetAnimation();
    void ToggleSneakWalk();
    bool IsAnimationPaused() const { return animationPaused_; }
    std::string GetCurrentAnimationName() const;
    
    // 状態取得
    bool IsMoving() const { return isMoving_; }
    bool IsSneaking() const { return isSneaking_; }
    bool IsBlending() const { return isBlending_; }
    float GetBlendProgress() const;
    
    // 回転スムーシング速度の設定
    void SetRotationSmoothingSpeed(float speed) { rotationSmoothingSpeed_ = speed; }
    float GetRotationSmoothingSpeed() const { return rotationSmoothingSpeed_; }
    float GetTargetRotationY() const { return targetRotationY_; }
    

private:
    void HandleMovement(UnoEngine* engine, float deltaTime);
    void UpdateAnimation(float deltaTime);
    void UpdateRotation(UnoEngine* engine, float deltaTime);
    
    // モデル関連
    std::unique_ptr<Object3d> object3d_;
    std::unique_ptr<AnimatedModel> animatedModel_;
    
    // 位置・回転
    Vector3 position_ = Vector3{0.0f, 0.0f, 0.0f};
    float currentRotationY_ = 0.0f;
    float targetRotationY_ = 0.0f;
    float rotationSmoothingSpeed_ = 17.0f;
    
    // 移動関連
    const float moveSpeed_ = 3.0f;  // 1秒あたり3ユニット（60FPSで0.05f = 1秒で3.0f）
    const float sneakSpeedMultiplier_ = 0.5f;
    bool isMoving_ = false;
    bool isSneaking_ = false;
    bool previousBButtonPressed_ = false;
    Vector3 moveDirection_ = Vector3{0.0f, 0.0f, 0.0f};
    
    // アニメーション関連
    bool animationPaused_ = false;
    bool isBlending_ = false;
    float blendTimer_ = 0.0f;
    const float BLEND_DURATION = 0.3f;
    
    // カメラ参照
    Camera* camera_ = nullptr;
};