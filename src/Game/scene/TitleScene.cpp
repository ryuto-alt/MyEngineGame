#include "TitleScene.h"
#include "../../Engine/Resource/ResourcePreloader.h"
#include "imgui.h"

TitleScene::TitleScene() {
    // コンストラクタでは特に何もしない
}

TitleScene::~TitleScene() {
    // デストラクタでも特に何もしない（ResourceはuniquePtr）
}

void TitleScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });

    // 3Dモデルとタイトルロゴの初期化をスキップして高速化
    // （必要最小限の初期化のみ）

    // リソースをプリロード（メインスレッドで実行）
    // OutputDebugStringA("TitleScene: Starting resource preload\n");
    // ResourcePreloader::GetInstance()->PreloadAnimatedModel("human", "Resources/Models/human", "sneakWalk.gltf", dxCommon_);
    // OutputDebugStringA("TitleScene: Resource preload completed\n");
    
    // 初期化完了
    initialized_ = true;
}

void TitleScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // カメラの更新
    camera_->Update();

    // SPACEキーでゲームプレイシーンへ
    if (input_->TriggerKey(DIK_SPACE)) {
        sceneManager_->ChangeScene("GamePlay");
    }
    
    // ESCキーで終了
    if (input_->TriggerKey(DIK_ESCAPE)) {
        exit(0);
    }
}

void TitleScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // タイトル画面を表示
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 200, ImGui::GetIO().DisplaySize.y * 0.3f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
    ImGui::Begin("Title", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("MY ENGINE GAME");
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Press SPACE to Start");
    ImGui::Text("Press ESC to Exit");
    
    ImGui::End();
}

void TitleScene::Finalize() {
    // 特に追加のリソース解放が必要なければ何もしない
}