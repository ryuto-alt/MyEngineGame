#include "TitleScene.h"
#include "../../Engine/Resource/ResourcePreloader.h"
#include "imgui.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

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

    // デバッグ出力
    OutputDebugStringA("TitleScene::Initialize - Starting initialization\n");
    OutputDebugStringA(input_ ? "TitleScene::Initialize - Input object is valid\n" : "TitleScene::Initialize - ERROR: Input object is null!\n");

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });

    // 3Dモデルとタイトルロゴの初期化をスキップして高速化
    // （必要最小限の初期化のみ）

    // リソースを軽量プリロード（高速起動）
    OutputDebugStringA("TitleScene: Starting lightweight resource preload\n");
    ResourcePreloader::GetInstance()->PreloadAnimatedModelLightweight("human_walk", "Resources/Models/human", "walk.gltf", dxCommon_);
    ResourcePreloader::GetInstance()->PreloadAnimatedModelLightweight("human_sneak", "Resources/Models/human", "sneakWalk.gltf", dxCommon_);
    
    float progress = ResourcePreloader::GetInstance()->GetPreloadProgress();
    OutputDebugStringA(("TitleScene: Lightweight preload completed (" + std::to_string(int(progress * 100)) + "%)\n").c_str());
    
    // 初期化完了
    initialized_ = true;
}

void TitleScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // カメラの更新
    camera_->Update();

    // Win32 APIを使った代替キー入力検出（ImGuiの影響を受けない）
    static bool spaceWasPressed = false;
    static bool escWasPressed = false;
    
    bool spaceIsPressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    bool escIsPressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    
    // スペースキーのトリガー検出（押した瞬間を検出）
    if (spaceIsPressed && !spaceWasPressed) {
        OutputDebugStringA("TitleScene: SPACE key triggered via Win32 API! Changing to GamePlay scene\n");
        sceneManager_->ChangeScene("GamePlay");
    }
    
    // ESCキーのトリガー検出
    if (escIsPressed && !escWasPressed) {
        OutputDebugStringA("TitleScene: ESC key triggered via Win32 API! Exiting game\n");
        exit(0);
    }
    
    // 前フレームの状態を保存
    spaceWasPressed = spaceIsPressed;
    escWasPressed = escIsPressed;

    // デバッグ：入力状態を監視
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) { // 1秒ごとに出力
        OutputDebugStringA("TitleScene: Waiting for input...\n");
        if (spaceIsPressed) {
            OutputDebugStringA("TitleScene: SPACE key is being held down (Win32)\n");
        }
        if (input_) {
            OutputDebugStringA("TitleScene: DirectInput object is valid\n");
            if (input_->PushKey(DIK_SPACE)) {
                OutputDebugStringA("TitleScene: SPACE key detected via DirectInput\n");
            }
        } else {
            OutputDebugStringA("TitleScene: ERROR - DirectInput object is null!\n");
        }
    }
}

void TitleScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // タイトル画面を表示
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 200, ImGui::GetIO().DisplaySize.y * 0.3f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_Always);
    ImGui::Begin("Title", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("MY ENGINE GAME");
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Press SPACE to Start GamePlay Scene");
    ImGui::Text("(Red background will be displayed)");
    ImGui::Text("Press ESC to Exit");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Input Status:");
    
    // リアルタイムでキー入力状態を表示
    bool spacePressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    bool escPressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    
    ImGui::Text("SPACE: %s", spacePressed ? "PRESSED" : "Not pressed");
    ImGui::Text("ESC: %s", escPressed ? "PRESSED" : "Not pressed");
    
    ImGui::End();
}

void TitleScene::Finalize() {
    // 特に追加のリソース解放が必要なければ何もしない
}