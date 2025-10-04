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
    float waitTimer_ = 0.0f;

    enum class FadeState {
        Wait,      // 待機状態
        FadeIn,
        Display,
        FadeOut,
        Complete
    };
    FadeState fadeState_ = FadeState::Wait;

    // フェード設定
    const float kWaitDuration = 1.5f;     // 待機時間
    const float kFadeInDuration = 1.0f;   // フェードイン時間
    const float kDisplayDuration = 2.0f;  // 表示時間
    const float kFadeOutDuration = 1.0f;  // フェードアウト時間

    bool soundPlayed_ = false;  // サウンド再生フラグ
};
