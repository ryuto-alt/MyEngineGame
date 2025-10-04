#include "LogoScene.h"
#include "SceneManager.h"
#include "AudioManager.h"

void LogoScene::Initialize() {
    OutputDebugStringA("LogoScene::Initialize() start\n");

    // 画面クリアカラーを黒に設定
    dxCommon_->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    camera_->SetTranslate({0.0f, 0.0f, -10.0f});

    // ロゴサウンドの読み込み（再生は1.5秒後）
    AudioManager::GetInstance()->LoadMP3("logoSound", "Resources/Audio/logosound.mp3");
    AudioManager::GetInstance()->SetVolume("logoSound", 0.5f);

    // ロゴスプライトの初期化
    logoSprite_ = std::make_unique<Sprite>();
    OutputDebugStringA("LogoScene: Loading logo.png\n");
    logoSprite_->Initialize(spriteCommon_, "Resources/textures/logo/logo.png");
    OutputDebugStringA("LogoScene: Logo loaded successfully\n");

    // 画面中央に配置
    logoSprite_->SetPosition({ 640.0f, 360.0f });
    logoSprite_->SetAnchorPoint({ 0.5f, 0.5f });

    // 初期状態は完全に透明
    logoSprite_->setColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    fadeTimer_ = 0.0f;
    displayTimer_ = 0.0f;
    waitTimer_ = 0.0f;
    fadeState_ = FadeState::Wait;
    soundPlayed_ = false;

    OutputDebugStringA("LogoScene::Initialize() completed\n");
}

void LogoScene::Update() {
    camera_->Update();

    float deltaTime = 1.0f / 60.0f;

    switch (fadeState_) {
    case FadeState::Wait:
        waitTimer_ += deltaTime;
        if (waitTimer_ >= kWaitDuration) {
            // 1.5秒経過したのでサウンド再生とフェードイン開始
            if (!soundPlayed_) {
                AudioManager::GetInstance()->Play("logoSound", false);
                soundPlayed_ = true;
            }
            fadeState_ = FadeState::FadeIn;
            fadeTimer_ = 0.0f;
        }
        break;

    case FadeState::FadeIn:
        fadeTimer_ += deltaTime;
        {
            float alpha = fadeTimer_ / kFadeInDuration;
            if (alpha >= 1.0f) {
                alpha = 1.0f;
                fadeState_ = FadeState::Display;
                fadeTimer_ = 0.0f;
            }
            logoSprite_->setColor({ 1.0f, 1.0f, 1.0f, alpha });
        }
        break;

    case FadeState::Display:
        displayTimer_ += deltaTime;
        if (displayTimer_ >= kDisplayDuration) {
            fadeState_ = FadeState::FadeOut;
            fadeTimer_ = 0.0f;
        }
        break;

    case FadeState::FadeOut:
        fadeTimer_ += deltaTime;
        {
            float alpha = 1.0f - (fadeTimer_ / kFadeOutDuration);
            if (alpha <= 0.0f) {
                alpha = 0.0f;
                fadeState_ = FadeState::Complete;
            }
            logoSprite_->setColor({ 1.0f, 1.0f, 1.0f, alpha });
        }
        break;

    case FadeState::Complete:
        // タイトルシーンへ遷移
        sceneManager_->ChangeScene("Title");
        break;
    }

    // スペースキーでスキップ
    if (input_->TriggerKey(DIK_SPACE) || input_->TriggerKey(DIK_RETURN)) {
        sceneManager_->ChangeScene("Title");
    }

    logoSprite_->Update();
}

void LogoScene::Draw() {
    // スプライト共通描画設定
    spriteCommon_->CommonDraw();

    // ロゴ描画
    logoSprite_->Draw();
}

void LogoScene::Finalize() {
    // ロゴサウンドの停止
    AudioManager::GetInstance()->Stop("logoSound");

    if (logoSprite_) {
        logoSprite_.reset();
    }
}
