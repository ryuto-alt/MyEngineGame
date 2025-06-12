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
    
    // 3D空間オーディオ
    std::unique_ptr<SpatialAudioSource> cubeSpatialAudio_;
    
    // キューブの位置
    Vector3 cubePosition_ = Vector3{0.0f, 0.0f, 0.0f};
};