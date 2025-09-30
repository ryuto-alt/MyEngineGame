#pragma once
#include "IScene.h"

class TitleScene : public IScene {
public:
    TitleScene() = default;
    ~TitleScene() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override {}
};