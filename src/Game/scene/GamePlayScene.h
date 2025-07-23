#pragma once
#include "UnoEngine.h"
#include "Player.h"
#include "Ground.h"
#include "LightManager.h"
#include "Skybox.h"
#include "TextureManager.h"
#include "OffscreenRenderingManager.h"
#include <vector>
#include <memory>


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
    std::unique_ptr<Skybox> skybox_;
    
    // オフスクリーンレンダリング
    std::unique_ptr<OffscreenRenderingManager> offscreenRenderingManager_;
    
    // GLBモデル用（テスト用キューブ）- マルチマテリアル対応
    std::vector<std::unique_ptr<Object3d>> cubeGlbObjects_;
    std::vector<std::unique_ptr<Model>> cubeGlbModels_;
    
    // Skybox制御フラグ
    bool skyboxEnabled_ = false;
    
    // カメラ移動速度（1秒あたりのユニット数）
    const float cameraSpeed_ = 0.6f;  // 60FPSで0.01f = 1秒で0.6f

private:
};