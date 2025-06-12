#include "GamePlayScene.h"

GamePlayScene::GamePlayScene() {
    // コンストラクタでは特に何もしない
}

GamePlayScene::~GamePlayScene() {
    // デストラクタでも特に何もしない
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期設定
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });
    camera_->Update();

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: 初期化完了（シンプル版）\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }

    // カメラの更新
    camera_->Update();
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // シンプルなImGuiウィンドウを表示
    ImGui::Begin("シンプルGamePlayScene");
    
    ImGui::Text("これはシンプルなGamePlaySceneです");
    ImGui::Text("ESCキーでタイトルに戻ります");
    
    ImGui::Separator();
    
    ImGui::Text("カメラ位置:");
    Vector3 cameraPos = camera_->GetTranslate();
    ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
    
    ImGui::End();
}

void GamePlayScene::Finalize() {
    OutputDebugStringA("GamePlayScene: 終了処理完了（シンプル版）\n");
}
