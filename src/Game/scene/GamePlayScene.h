#pragma once
#include "IScene.h"
#include "GameObject/Player.h"
#include "GameObject/Ground.h"
#include "Skybox.h"
#include "Manager/LightManager.h"
#include <memory>

class GamePlayScene : public IScene {
public:
    GamePlayScene() = default;
    ~GamePlayScene() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    void HandleInput();
    void UpdateCamera();
    void DrawUI();

    std::unique_ptr<Player> player_;
    std::unique_ptr<Ground> ground_;
    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<LightManager> lightManager_;

    std::unique_ptr<Object3d> objeObject_;
    std::unique_ptr<AnimatedModel> objeModel_;

    bool skyboxEnabled_ = false;
};