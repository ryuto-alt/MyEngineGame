#pragma once
#include "IScene.h"
#include "Sprite.h"
#include "PostProcess.h"
#include <memory>

class TitleScene : public IScene {
public:
    TitleScene() = default;
    ~TitleScene() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    std::unique_ptr<Sprite> titleBgSprite_;
    std::unique_ptr<Sprite> titleTextSprite_;
    std::unique_ptr<PostProcess> horrorEffect_;
    float time_ = 0.0f;
};