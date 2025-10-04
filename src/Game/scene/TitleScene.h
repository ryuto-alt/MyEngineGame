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
    std::unique_ptr<Sprite> titleBg2Sprite_;
    std::unique_ptr<Sprite> titleTextSprite_;
    std::unique_ptr<Sprite> hazimeruSprite_;
    std::unique_ptr<Sprite> owaruSprite_;
    std::unique_ptr<PostProcess> horrorEffect_;
    float time_ = 0.0f;

    // メニュー選択
    enum class MenuSelection {
        Start = 0,  // はじめる
        Exit = 1    // おわる
    };
    MenuSelection currentSelection_ = MenuSelection::Start;
    bool CheckMouseHover(const Vector2& mousePos, const Vector2& spritePos, const Vector2& spriteSize);

    // 選択エフェクト用
    Vector2 hazimeruOriginalSize_;
    Vector2 owaruOriginalSize_;
    float noiseTimer_ = 0.0f;
};