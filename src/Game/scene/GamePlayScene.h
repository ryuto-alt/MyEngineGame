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
    
    // コントローラー操作関連
    bool isMoving_ = false;
    bool isSneaking_ = false;
    bool previousBButtonPressed_ = false;
    Vector3 moveDirection_ = Vector3{0.0f, 0.0f, 0.0f};
    float currentRotationY_ = 0.0f;
    
    // シンプルなブレンド管理
    bool isBlending_ = false;
    float blendTimer_ = 0.0f;
    const float BLEND_DURATION = 0.3f;
};