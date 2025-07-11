#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "AnimatedModel.h"
#include "UnoEngine.h"
#include <memory>

class AnimatedHumanController {
public:
    AnimatedHumanController();
    
    ~AnimatedHumanController();

    void Initialize(DirectXCommon* dxCommon);

    void Update();

    void Draw();

    void Finalize();

    // セッター
    void SetPosition(const Vector3& position) { position_ = position; }
    void SetRotation(const Vector3& rotation) { rotation_ = rotation; }
    void SetScale(const Vector3& scale) { scale_ = scale; }
    void SetColor(const Vector4& color) { color_ = color; }
    void SetAnimationSpeed(float speed) { animationSpeed_ = speed; }
    void SetAnimationPaused(bool paused) { animationPaused_ = paused; }

    // ゲッター
    Vector3 GetPosition() const { return position_; }
    Vector3 GetRotation() const { return rotation_; }
    Vector3 GetScale() const { return scale_; }
    Vector4 GetColor() const { return color_; }
    float GetAnimationSpeed() const { return animationSpeed_; }
    bool IsAnimationPaused() const { return animationPaused_; }
    float GetAnimationTime() const { return animationTime_; }

    void ResetAnimation();

    void ToggleAnimation();

private:
    // アニメーション付きモデル
    std::unique_ptr<AnimatedModel> animatedModel_;
    std::unique_ptr<Object3d> object3d_;
    
    // トランスフォーム
    Vector3 position_;
    Vector3 rotation_;
    Vector3 scale_;
    Vector4 color_;
    
    // アニメーション関連
    float animationTime_;
    float animationSpeed_;
    bool animationPaused_;
    float animationDuration_;
    
    // 初期化フラグ
    bool initialized_;
    
    // DirectXCommon
    DirectXCommon* dxCommon_;
    
    // UnoEngine
    UnoEngine* engine_;
};