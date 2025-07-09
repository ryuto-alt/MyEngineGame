#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "AnimatedModel.h"
#include "UnoEngine.h"
#include <memory>

/// <summary>
/// アニメーション付きヒューマンモデルを制御するクラス
/// walk.gltfのアニメーションを管理
/// </summary>
class AnimatedHumanController {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    AnimatedHumanController();
    
    /// <summary>
    /// デストラクタ
    /// </summary>
    ~AnimatedHumanController();

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="dxCommon">DirectX共通クラス</param>
    void Initialize(DirectXCommon* dxCommon);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// 終了処理
    /// </summary>
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

    /// <summary>
    /// アニメーションをリセット
    /// </summary>
    void ResetAnimation();

    /// <summary>
    /// アニメーションの一時停止/再開
    /// </summary>
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