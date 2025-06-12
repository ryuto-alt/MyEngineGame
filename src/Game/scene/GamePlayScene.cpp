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
    engine_->SetCameraPosition(Vector3{ 0.0f, 0.0f, -10.0f });

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
    
    // キューブの初期位置を設定
    cubePosition_ = Vector3{0.0f, 0.0f, 0.0f};
    cubeObject_->SetPosition(cubePosition_);

    // 2Dスプライトの作成
    titleSprite_ = engine_->CreateSprite("Resources/textures/title_logo.png");
    titleSprite_->SetPosition({100.0f, 50.0f});
    titleSprite_->SetSize({200.0f, 100.0f});

    // 3D空間オーディオの作成（キューブの位置に音源を配置）
    if (engine_->LoadAudio("cube_bgm", "Resources/Audio/bgm.wav")) {
        cubeSpatialAudio_ = engine_->CreateSpatialAudioSource("cube_bgm", cubePosition_);
        if (cubeSpatialAudio_) {
            cubeSpatialAudio_->SetMaxDistance(20.0f);  // 20ユニット以内で聞こえる
            cubeSpatialAudio_->SetMinDistance(2.0f);   // 2ユニット以内では減衰なし
            cubeSpatialAudio_->SetVolume(0.7f);        // 音量70%
        }
    }

    // 初期化完了
    initialized_ = true;
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
        cubeObject_->SetRotation(Vector3{0.0f, rotation, 0.0f});
    }

    // パーティクル発生（統合APIを使用）
    if (engine_->IsKeyTriggered(DIK_F)) {
        engine_->PlayParticle("explosion", Vector3{0.0f, 2.0f, 0.0f}, 20);
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
    
    // 十字キーでキューブ移動
    const float cubeSpeed = 0.05f;
    
    if (engine_->IsKeyPressed(DIK_UP)) cubePosition_.z += cubeSpeed;
    if (engine_->IsKeyPressed(DIK_DOWN)) cubePosition_.z -= cubeSpeed;
    if (engine_->IsKeyPressed(DIK_LEFT)) cubePosition_.x -= cubeSpeed;
    if (engine_->IsKeyPressed(DIK_RIGHT)) cubePosition_.x += cubeSpeed;
    
    // キューブの位置を更新
    cubeObject_->SetPosition(cubePosition_);
    
    // 3D空間オーディオリスナーの位置をカメラと同期
    engine_->SetAudioListenerPosition(currentPos);
    
    // 3D空間オーディオソースの操作
    if (cubeSpatialAudio_) {
        // Cキーで3D音源再生/停止
        if (engine_->IsKeyTriggered(DIK_C)) {
            if (cubeSpatialAudio_->IsPlaying()) {
                cubeSpatialAudio_->Stop();
            } else {
                cubeSpatialAudio_->Play(true); // ループ再生
            }
        }
        
        // キューブの位置に音源を更新
        cubeSpatialAudio_->SetPosition(cubePosition_);
        
        // 3D空間オーディオを更新
        cubeSpatialAudio_->Update(currentPos, Vector3{0.0f, 0.0f, 1.0f});
    } else {
        // 3D空間オーディオが無効な場合は直接再生
        if (engine_->IsKeyTriggered(DIK_C)) {
            if (engine_->IsAudioPlaying("cube_bgm")) {
                engine_->StopAudio("cube_bgm");
            } else {
                engine_->PlayAudio("cube_bgm", true);
            }
        }
    }

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
    ImGui::Text("C - bgm.wav再生/停止（3D空間オーディオ）");
    ImGui::Text("WASD - カメラ移動");
    ImGui::Text("↑↓←→ - キューブ移動（音源移動）");
    ImGui::Text("ESC - タイトルに戻る");
    
    ImGui::Separator();
    
    // 衝突判定テスト
    Vector3 playerPos = Vector3{0.0f, 0.0f, 0.0f};
    Vector3 enemyPos = Vector3{1.0f, 0.0f, 0.0f};
    bool collision = engine_->CheckCollision(playerPos, 0.5f, enemyPos, 0.5f);
    ImGui::Text("衝突判定テスト: %s", collision ? "衝突中" : "衝突していません");
    
    ImGui::Separator();
    
    // 3D空間オーディオ情報
    ImGui::Text("3D空間オーディオ:");
    ImGui::Text("cubeSpatialAudio_: %s", cubeSpatialAudio_ ? "有効" : "NULL");
    
    if (cubeSpatialAudio_) {
        Vector3 cubePos = cubeSpatialAudio_->GetPosition();
        Vector3 cameraPos = engine_->GetCameraPosition();
        float distance = cubeSpatialAudio_->GetDistanceToListener();
        
        ImGui::Text("キューブ音源位置: (%.1f, %.1f, %.1f)", cubePosition_.x, cubePosition_.y, cubePosition_.z);
        ImGui::Text("カメラ位置: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("リスナー距離: %.2f", distance);
        ImGui::Text("bgm.wav再生中: %s", cubeSpatialAudio_->IsPlaying() ? "Yes" : "No");
        ImGui::Text("最大距離: %.1f, 最小距離: %.1f", 20.0f, 2.0f);
        ImGui::Text("カメラとキューブの距離が近いほどbgm.wavが大きく聞こえます");
        ImGui::Text("十字キーでキューブを移動して音の位置変化をテスト");
    } else {
        ImGui::Text("3D音源なし - 初期化失敗の可能性");
        ImGui::Text("音楽ファイルの読み込みまたは音源作成に失敗");
    }
    
    ImGui::End();

    // デバッグ情報の表示（統合APIを使用）
    engine_->ShowDebugInfo();
}

void GamePlayScene::Finalize() {
    // 3D空間オーディオの停止と解放
    if (cubeSpatialAudio_) {
        cubeSpatialAudio_->Stop();
        cubeSpatialAudio_.reset();
    }
    
    cubeObject_.reset();
    cubeModel_.reset();
    titleSprite_.reset();
}
