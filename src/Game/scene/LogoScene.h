#pragma once
#include "IScene.h"
#include "Sprite.h"
#include <memory>

class LogoScene : public IScene {
public:
    LogoScene() = default;
    ~LogoScene() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    std::unique_ptr<Sprite> logoSprite_;
    float fadeTimer_ = 0.0f;
    float displayTimer_ = 0.0f;

    enum class FadeState {
        FadeIn,
        Display,
        FadeOut,
        Complete
    };
    FadeState fadeState_ = FadeState::FadeIn;

    // フェード設定
    const float kFadeInDuration = 1.0f;   // フェードイン時間
    const float kDisplayDuration = 2.0f;  // 表示時間
    const float kFadeOutDuration = 1.0f;  // フェードアウト時間
};
