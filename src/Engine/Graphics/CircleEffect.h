#pragma once
#include "Ring.h"
#include "Object3d.h"
#include "Model.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Camera.h"
#include <memory>

/// <summary>
/// 円形エフェクトクラス
/// gradationLine.pngテクスチャとRingプリミティブを組み合わせて
/// 画像で説明されたような円形エフェクトを描画する
/// </summary>
class CircleEffect {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CircleEffect();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CircleEffect();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectXCommon</param>
    /// <param name="spriteCommon">SpriteCommon</param>
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);

    /// <summary>
    /// 円形エフェクトの作成
    /// </summary>
    /// <param name="outerRadius">外径</param>
    /// <param name="innerRadius">内径</param>
    /// <param name="divisions">分割数</param>
    void CreateCircleEffect(float outerRadius = 1.0f, float innerRadius = 0.2f, uint32_t divisions = 32);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// <summary>
    /// カメラの設定
    /// </summary>
    /// <param name="camera">Camera</param>
    void SetCamera(Camera* camera);

    // Transform関連
    void SetPosition(const Vector3& position);
    void SetRotation(const Vector3& rotation);
    void SetScale(const Vector3& scale);
    void SetColor(const Vector4& color);

    // UVScroll関連
    void SetUVScroll(float scrollU, float scrollV);
    void StartUVAnimation(float speedU = 0.01f, float speedV = 0.0f);
    void StopUVAnimation();

    // エフェクト制御
    void SetVisible(bool visible) { isVisible_ = visible; }
    bool IsVisible() const { return isVisible_; }

    // アクセサ
    const Vector3& GetPosition() const;
    const Vector3& GetRotation() const;
    const Vector3& GetScale() const;

private:
    DirectXCommon* dxCommon_;
    SpriteCommon* spriteCommon_;
    
    std::unique_ptr<Ring> ring_;
    std::unique_ptr<Object3d> object3d_;
    
    // UVアニメーション関連
    bool isUVAnimating_;
    float uvScrollSpeedU_;
    float uvScrollSpeedV_;
    float currentUVScrollU_;
    float currentUVScrollV_;
    
    // エフェクト状態
    bool isVisible_;
    float effectTime_;
};
