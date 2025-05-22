#pragma once
#include "IScene.h"
#include "MagicCircleEffect.h"
#include "PortalEffect.h"
#include <memory>
#include <vector>

/// <summary>
/// ゲームプレイシーン - エピックエフェクトデモ
/// ゲーム品質の派手なエフェクトを2種類展示
/// </summary>
class GamePlayScene : public IScene {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    GamePlayScene();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~GamePlayScene();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    /// <summary>
    /// エフェクト切り替え処理
    /// </summary>
    void HandleEffectSwitching();

    /// <summary>
    /// 動的エフェクト更新
    /// </summary>
    /// <param name="deltaTime">デルタタイム</param>
    void UpdateDynamicEffects(float deltaTime);

    /// <summary>
    /// エフェクトUI表示
    /// </summary>
    void ShowEffectUI();

    // エフェクトオブジェクト
    std::unique_ptr<MagicCircleEffect> magicCircleEffect_;
    std::unique_ptr<PortalEffect> portalEffect_;

    // アニメーション制御
    float animationTime_;
    float effectTimer_;
    int currentEffectMode_; // 0: 両方, 1: 魔法陣, 2: ポータル
    bool keyPressed_;
    
    // エフェクト表示状態制御
    bool magicCircleVisible_;
    bool portalVisible_;

    // 初期化フラグ
    bool initialized_ = false;
};
