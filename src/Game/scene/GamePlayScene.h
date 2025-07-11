#pragma once
#include "UnoEngine.h"


class GamePlayScene : public IScene {
public:
    // コンストラクタ・デストラクタ
    GamePlayScene();
    ~GamePlayScene() override;

    // ISceneの実装
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

protected:
    // 初期化済みフラグ
    bool initialized_ = false;
    
    // モデル読み込み完了フラグ
    bool modelLoaded_ = false;
    bool groundLoaded_ = false;
    
    // UnoEngineインスタンス
    UnoEngine* engine_ = nullptr;

    std::unique_ptr<Object3d> humanObject3d_;
    std::unique_ptr<AnimatedModel> humanAnimatedModel_;
    
    // Groundモデル
    std::unique_ptr<Object3d> groundObject3d_;
    std::unique_ptr<Model> groundModel_;
    
    // アニメーション制御
    float animationTime_ = 0.0f;
    bool animationPaused_ = false;
    bool enableAnimation_ = true;
    
    // 移動速度
    const float moveSpeed_ = 0.01f;
    const float humanSpeed_ = 0.05f;
};