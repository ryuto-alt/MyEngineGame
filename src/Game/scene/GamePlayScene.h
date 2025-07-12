#pragma once
#include "UnoEngine.h"


class GamePlayScene : public IScene {
public:
    GamePlayScene();
    ~GamePlayScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

protected:
    bool initialized_ = false;
    bool modelLoaded_ = false;
    bool groundLoaded_ = false;
    
    UnoEngine* engine_ = nullptr;

    std::unique_ptr<Object3d> humanObject3d_;
    std::unique_ptr<AnimatedModel> humanAnimatedModel_;
    std::unique_ptr<Object3d> groundObject3d_;
    std::unique_ptr<Model> groundModel_;
    
    float animationTime_ = 0.0f;
    bool animationPaused_ = false;
    bool enableAnimation_ = true;
    
    const float moveSpeed_ = 0.01f;
    const float humanSpeed_ = 0.05f;
    
    bool isMoving_ = false;
    bool isSneaking_ = false;
    bool previousBButtonPressed_ = false;
    Vector3 moveDirection_ = Vector3{0.0f, 0.0f, 0.0f};
    float currentRotationY_ = 0.0f;
    float targetRotationY_ = 0.0f;
    float rotationSmoothingSpeed_ = 17.0f;  // 回転スムーシング速度
    
    bool isBlending_ = false;
    float blendTimer_ = 0.0f;
    const float BLEND_DURATION = 0.3f;

private:
    // 角度を-π～πの範囲に正規化
    float NormalizeAngle(float angle);
    // 角度の最短距離を計算
    float AngleDifference(float from, float to);
    // 角度を線形補間
    float LerpAngle(float from, float to, float t);
};