#include "GamePlayScene.h"

GamePlayScene::GamePlayScene() {
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // UnoEngineインスタンスを取得
    engine_ = UnoEngine::GetInstance();

    // カメラの初期設定（統合APIを使用）
    engine_->SetCameraPosition({ 0.0f, 0.0f, -10.0f });

    // ゲームリソースの読み込み（統合APIを使用）
    // BGMの読み込み（失敗してもエラーにしない）
    if (!engine_->LoadAudio("bgm", "Resources/Audio/bgm.wav")) {
        OutputDebugStringA("警告: BGMファイルの読み込みに失敗しました。BGM機能は無効になります。\n");
    }
    engine_->CreateParticleEffect("explosion", "Resources/particle/explosion.png");
    
    // 3Dオブジェクトの作成
    cubeObject_ = engine_->CreateObject3D();
    cubeModel_ = engine_->LoadModel("Resources/Models/cube/cube.obj");
    cubeObject_->SetModel(cubeModel_.get());
    cubeObject_->SetPosition({0.0f, 0.0f, 0.0f});

    // 2Dスプライトの作成
    titleSprite_ = engine_->CreateSprite("Resources/textures/title_logo.png");
    titleSprite_->SetPosition({100.0f, 50.0f});
    titleSprite_->SetSize({200.0f, 100.0f});

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: 初期化完了（統合API使用版）\n");
}

void GamePlayScene::Update() {
    if (!initialized_) return;

    // ESCキーでタイトルシーンに戻る（統合APIを使用）
    if (engine_->IsKeyTriggered(DIK_ESCAPE)) {
        engine_->ChangeScene("Title");
    }

    // キューブの回転（統合APIを使用）
    if (engine_->IsKeyPressed(DIK_SPACE)) {
        static float rotation = 0.0f;
        rotation += 0.02f;
        cubeObject_->SetRotation({0.0f, rotation, 0.0f});
    }

    // パーティクル発生（統合APIを使用）
    if (engine_->IsKeyTriggered(DIK_F)) {
        engine_->PlayParticle("explosion", {0.0f, 2.0f, 0.0f}, 20);
    }

    // BGM制御（統合APIを使用）
    if (engine_->IsKeyTriggered(DIK_B)) {
        try {
            if (engine_->IsAudioPlaying("bgm")) {
                engine_->StopAudio("bgm");
            } else {
                engine_->PlayAudio("bgm", true);
            }
        } catch (...) {
            // BGMの操作でエラーが発生しても続行
            OutputDebugStringA("BGM操作でエラーが発生しましたが、ゲームを続行します。\n");
        }
    }

    // カメラのWASD移動（統合APIを使用）
    Vector3 currentPos = engine_->GetCameraPosition();
    const float moveSpeed = 0.1f;
    
    if (engine_->IsKeyPressed(DIK_W)) currentPos.z += moveSpeed;
    if (engine_->IsKeyPressed(DIK_S)) currentPos.z -= moveSpeed;
    if (engine_->IsKeyPressed(DIK_A)) currentPos.x -= moveSpeed;
    if (engine_->IsKeyPressed(DIK_D)) currentPos.x += moveSpeed;
    
    engine_->SetCameraPosition(currentPos);

    // オブジェクトの更新
    cubeObject_->Update();
    titleSprite_->Update();

    // カメラの更新
    camera_->Update();
}

void GamePlayScene::Draw() {
    if (!initialized_) return;

    // 3Dオブジェクトの描画
    cubeObject_->Draw();

    // 2Dスプライトの描画
    titleSprite_->Draw();

    // GamePlayScene用のImGuiウィンドウ
    ImGui::Begin("GamePlayScene - 統合API版");
    
    ImGui::Text("統合APIを使用したGamePlaySceneです");
    ImGui::Separator();
    
    ImGui::Text("操作方法:");
    ImGui::Text("SPACE - キューブ回転");
    ImGui::Text("F - パーティクル発生");
    ImGui::Text("B - BGM ON/OFF");
    ImGui::Text("WASD - カメラ移動");
    ImGui::Text("ESC - タイトルに戻る");
    
    ImGui::Separator();
    
    // 衝突判定テスト
    Vector3 playerPos = {0.0f, 0.0f, 0.0f};
    Vector3 enemyPos = {1.0f, 0.0f, 0.0f};
    bool collision = engine_->CheckCollision(playerPos, 0.5f, enemyPos, 0.5f);
    ImGui::Text("衝突判定テスト: %s", collision ? "衝突中" : "衝突していません");
    
    ImGui::End();

    // デバッグ情報の表示（統合APIを使用）
    engine_->ShowDebugInfo();
}

void GamePlayScene::Finalize() {
    cubeObject_.reset();
    cubeModel_.reset();
    titleSprite_.reset();
    OutputDebugStringA("GamePlayScene: 終了処理完了（統合API使用版）\n");
}
