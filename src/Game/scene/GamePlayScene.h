#pragma once
#include "UnoEngine.h"
#include "../Player.h"
#include "../Ground.h"
#include "../LightManager.h"


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
    
    UnoEngine* engine_ = nullptr;

    // ゲームオブジェクト
    std::unique_ptr<Player> player_;
    std::unique_ptr<Ground> ground_;
    std::unique_ptr<LightManager> lightManager_;
    
    // GLBモデル用（テスト用キューブ）
    std::unique_ptr<Object3d> cubeGlbObject3d_;
    std::unique_ptr<Model> cubeGlbModel_;
    
    // カメラ移動速度（1秒あたりのユニット数）
    const float cameraSpeed_ = 0.6f;  // 60FPSで0.01f = 1秒で0.6f

private:
};