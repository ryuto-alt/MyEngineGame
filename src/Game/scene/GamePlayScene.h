#pragma once
#include "UnoEngine.h"

// 統合APIを使用したGamePlayScene
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
    
    // UnoEngineインスタンス
    UnoEngine* engine_ = nullptr;
    
    // ゲームオブジェクト
    std::unique_ptr<Object3d> cubeObject_;
    std::unique_ptr<Model> cubeModel_;
    std::unique_ptr<Sprite> titleSprite_;
};