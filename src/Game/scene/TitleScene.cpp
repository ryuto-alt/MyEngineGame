#include "TitleScene.h"
#include "../../Engine/Resource/ResourcePreloader.h"
#include "SceneManager.h"
#include "imgui.h"

void TitleScene::Initialize() {
    camera_->SetTranslate({0.0f, 0.0f, -10.0f});

    // ホラーエフェクトの初期化
    horrorEffect_ = std::make_unique<PostProcess>();
    horrorEffect_->Initialize(dxCommon_, srvManager_);

    // タイトルスプライトの初期化（通常の色で）
    titleBgSprite_ = std::make_unique<Sprite>();
    titleBgSprite_->Initialize(spriteCommon_, "Resources/textures/Title/Title_bg.png");

    titleTextSprite_ = std::make_unique<Sprite>();
    titleTextSprite_->Initialize(spriteCommon_, "Resources/textures/Title/Title_moji.png");

    ResourcePreloader::GetInstance()->PreloadAnimatedModelLightweight("human_walk", "Resources/Models/human", "walk.gltf", dxCommon_);
    ResourcePreloader::GetInstance()->PreloadAnimatedModelLightweight("human_sneak", "Resources/Models/human", "sneakWalk.gltf", dxCommon_);

    // タイトルBGMの読み込みと再生
    AudioManager::GetInstance()->LoadMP3("titleBGM", "Resources/Audio/title.mp3");
    AudioManager::GetInstance()->SetVolume("titleBGM", 0.2f);
    AudioManager::GetInstance()->Play("titleBGM", true);
}

void TitleScene::Update() {
    camera_->Update();

    // スプライトの更新
    titleBgSprite_->Update();
    titleTextSprite_->Update();

    // ホラーエフェクトのパラメータ更新
    time_ += 1.0f / 60.0f;
    horrorEffect_->SetHorrorParams(
        time_,
        0.4f,  // ノイズ強度
        0.6f,  // 歪み強度
        0.3f   // 血エフェクト強度
    );

    if (input_->TriggerKey(DIK_SPACE)) {
        sceneManager_->ChangeScene("GamePlay");
    }

    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->RequestExit();
    }
}

void TitleScene::Draw() {
    // ホラーエフェクトのレンダーターゲットに描画開始
    horrorEffect_->PreDraw();

    // スプライト共通描画設定
    spriteCommon_->CommonDraw();

    // 背景だけエフェクトのレンダーターゲットに描画
    titleBgSprite_->Draw();

    // ホラーエフェクトを適用してバックバッファに描画
    horrorEffect_->PostDraw();

    // エフェクト適用後、タイトル文字をバックバッファに直接描画
    spriteCommon_->CommonDraw();
    titleTextSprite_->Draw();
}

void TitleScene::Finalize() {
    OutputDebugStringA("TitleScene::Finalize() called\n");

    // タイトルBGMの停止
    AudioManager::GetInstance()->Stop("titleBGM");

    // スプライトの解放
    if (titleBgSprite_) {
        OutputDebugStringA("  Releasing titleBgSprite_\n");
        titleBgSprite_.reset();
    }
    if (titleTextSprite_) {
        OutputDebugStringA("  Releasing titleTextSprite_\n");
        titleTextSprite_.reset();
    }

    // ホラーエフェクトの明示的な解放
    if (horrorEffect_) {
        OutputDebugStringA("  Finalizing horrorEffect_\n");
        horrorEffect_->Finalize();
        horrorEffect_.reset();
    }

    OutputDebugStringA("TitleScene::Finalize() completed\n");
}