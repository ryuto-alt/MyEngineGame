#include "GamePlayScene.h"
#include "TextureManager.h"

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
    camera_->SetTranslate({ 0.0f, 5.0f, -20.0f });
    camera_->SetRotate({ 0.1f, 0.0f, 0.0f });
    camera_->Update();
    
    // ステージエディターの初期化
    if (isDebugMode_) {
        OutputDebugStringA("GamePlayScene: Debug mode is ON, initializing StageEditor\n");
        stageEditor_ = std::make_unique<StageEditor>();
        stageEditor_->Initialize(camera_, input_, dxCommon_, spriteCommon_);
    } else {
        OutputDebugStringA("GamePlayScene: Debug mode is OFF\n");
    }

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: 初期化完了\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;
    
    // F1キーでエディターモード切り替え
    if (isDebugMode_ && input_->TriggerKey(DIK_F1)) {
        if (stageEditor_) {
            bool newState = !stageEditor_->IsEnabled();
            stageEditor_->SetEnabled(newState);
            if (newState) {
                OutputDebugStringA("StageEditor: ENABLED\n");
            } else {
                OutputDebugStringA("StageEditor: DISABLED\n");
            }
        }
    }
    
    // エディターモードでない場合のみESCキーでタイトルに戻る
    if (!stageEditor_ || !stageEditor_->IsEnabled()) {
        if (input_->TriggerKey(DIK_ESCAPE)) {
            sceneManager_->ChangeScene("Title");
        }
    }
    
    // ステージエディターの更新
    if (stageEditor_) {
        stageEditor_->Update();
    }

    // カメラの更新
    camera_->Update();
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;
    
    // ステージエディターのオブジェクト描画
    if (stageEditor_) {
        stageEditor_->DrawObjects();
    }

    // ImGuiでシーン情報のみ表示
    ImGui::Begin("GamePlayScene情報");
    
    ImGui::Text("シーン名: GamePlayScene");
    ImGui::Text("状態: 初期化済み");
    if (isDebugMode_) {
        ImGui::Text("Debug Mode: ON");
        ImGui::Text("F1: Toggle Stage Editor");
        if (stageEditor_ && stageEditor_->IsEnabled()) {
            ImGui::Text("Stage Editor: ACTIVE");
        }
    }
    ImGui::Separator();
    
    ImGui::Text("カメラ情報:");
    Vector3 camPos = camera_->GetTranslate();
    Vector3 camRot = camera_->GetRotate();
    ImGui::Text("位置: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camRot.z);
    ImGui::Text("回転: (%.2f, %.2f, %.2f)", camRot.x, camRot.y, camRot.z);
    ImGui::Separator();
    
    ImGui::Text("操作説明:");
    if (!stageEditor_ || !stageEditor_->IsEnabled()) {
        ImGui::Text("ESC - タイトルに戻る");
    }
    
    ImGui::End();
    
    // ステージエディターのImGui描画
    if (stageEditor_) {
        stageEditor_->DrawImGui();
    }
}

void GamePlayScene::Finalize() {
    stageEditor_.reset();
    OutputDebugStringA("GamePlayScene: 終了処理完了\n");
}