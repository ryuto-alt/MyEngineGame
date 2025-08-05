#include "GamePlayScene.h"
#ifdef _DEBUG
#include "imgui.h"
#endif


GamePlayScene::GamePlayScene() {
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    engine_ = UnoEngine::GetInstance();
    
    // ポストプロセスを有効化
    engine_->EnablePostProcessing(true);
    engine_->SetPostProcessMode(0); // Normalモード

    // カメラの初期設定
    engine_->SetCameraFov(1.37f);
    engine_->SetCameraPosition(Vector3{ 0.0f, 2.0f, -10.0f });
    engine_->SetCameraRotation(Vector3{ 0.1f, 0.0f, 0.0f });
    engine_->SetStickSensitivity(2.5f);
    
    // 追従カメラ設定
    engine_->SetCameraDistance(8.0f);
    engine_->SetCameraHeight(3.0f);
    engine_->SetCameraTarget(Vector3{0.0f, 0.0f, 0.0f});
    
    // 一人称視点モードに設定
    camera_->SetCameraMode(CAMERA_MODE_FIRST_PERSON);
    
    // マネージャーの初期化
    lightManager_ = std::make_unique<LightManager>();
    lightManager_->Initialize();

    // プレイヤーの作成と初期化
    player_ = std::make_unique<Player>();
    player_->Initialize(camera_);
    
    // グラウンドの作成と初期化
    ground_ = std::make_unique<Ground>();
    ground_->Initialize(camera_, dxCommon_);
    
    // スカイボックスのロード
    skyboxEnabled_ = engine_->LoadSkybox("Resources/Models/skybox/rostock_laage_airport_4k.dds");
    
    // オフスクリーンレンダリング専用モードで動作
    
    // GLBキューブモデルのロード
    cubeGlbObjects_ = engine_->LoadGLBModel("Resources/Models/cube/obje.glb", Vector3{ 3.0f, 1.0f, 2.0f });

    // Blender JSONシーンのロード
    engine_->LoadBlenderScene("Resources/blender/obje.json");

    initialized_ = true;
}

void GamePlayScene::Update() {
    if (!initialized_) return;

    // デルタタイムを取得
    float deltaTime = engine_->GetDeltaTime();

    // ESCキーで終了
    if (engine_->IsPause()) {
        engine_->RequestEnd();
    }

#ifdef _DEBUG
    // TABキーでマウス視点移動の切り替え
    if (engine_->IsKeyTriggered(DIK_TAB)) {
        camera_->ToggleMouseLook();
    }
    
    // F1キーでフリーカメラモード切り替え
    if (engine_->IsKeyTriggered(DIK_F1)) {
        camera_->ToggleFreeCameraMode();
    }
    
#endif

    // F2キーで一人称視点とオービットカメラモードの切り替え
    if (engine_->IsKeyTriggered(DIK_F2)) {
        if (camera_->GetCameraMode() == CAMERA_MODE_FIRST_PERSON) {
            camera_->SetCameraMode(CAMERA_MODE_ORBIT);
        } else if (camera_->GetCameraMode() == CAMERA_MODE_ORBIT) {
            camera_->SetCameraMode(CAMERA_MODE_FIRST_PERSON);
        }
    }

    // フィルター効果切り替え
    if (engine_->IsKeyTriggered(DIK_1)) {
        currentPostProcessMode_ = 0;
        engine_->SetPostProcessMode(0); // Normal
    }
    if (engine_->IsKeyTriggered(DIK_2)) {
        currentPostProcessMode_ = 1;
        engine_->SetPostProcessMode(1); // Grayscale
    }
    if (engine_->IsKeyTriggered(DIK_3)) {
        currentPostProcessMode_ = 2;
        engine_->SetPostProcessMode(2); // Vignetting
    }
    if (engine_->IsKeyTriggered(DIK_4)) {
        currentPostProcessMode_ = 3;
        engine_->SetPostProcessMode(3); // Horror
    }

    // カメラ入力処理（マウス + 右スティックを統合）
    engine_->UpdateCameraInput();
    
#ifdef _DEBUG
    // フリーカメラモードでない場合のみプレイヤー追従
    if (!engine_->IsFreeCameraMode()) {
#endif
        // カメラフォロー更新（プレイヤー移動処理の前に実行）
        player_->UpdateCameraFollow(deltaTime);
#ifdef _DEBUG
    }
#endif
    
    // カメラの行列を即座に更新（プレイヤー移動で正しい方向ベクトルを取得するため）
    camera_->Update();

    // プレイヤー移動（カメラの向きに基づく統合移動、デルタタイム考慮）
#ifdef _DEBUG
    // フリーカメラモードの移動処理
    engine_->UpdateFreeCameraMovement();
    
    if (!engine_->IsFreeCameraMode()) {
#endif
        // スプリント処理（SHIFT キー）
        bool isSprintPressed = engine_->IsKeyPressed(DIK_LSHIFT);
        player_->SetSprinting(isSprintPressed);
        
        // 統合移動入力を取得
        Vector2 movement = engine_->GetMovementInput();
        
        if (engine_->IsMoving()) {
            player_->MoveWithCameraDirection(movement.y, movement.x, deltaTime);
        } else if (player_->IsMoving()) {
            player_->StopMoving();
        }
#ifdef _DEBUG
    }
#endif

    // Fキーでライティングデバッグウィンドウの表示切り替え
    if (engine_->IsInteract()) {
        lightManager_->ToggleDebugDisplay();
    }

    // キーボード操作によるプレイヤーアニメーション制御
    if (engine_->IsKeyTriggered(DIK_P)) {
        if (player_->IsAnimationPaused()) {
            player_->PlayAnimation();
        } else {
            player_->PauseAnimation();
        }
    }
    if (engine_->IsKeyTriggered(DIK_R)) {
        player_->ResetAnimation();
    }
    // スニーク歩行の切り替え（数字キーと重複しないように変更）
    if (engine_->IsKeyTriggered(DIK_RSHIFT)) {
        player_->ToggleSneakWalk();
    }

    // ライトマネージャーの更新
    lightManager_->Update();
    
    // カメラモードに応じてスポットライトを更新
    if (camera_->GetCameraMode() == CAMERA_MODE_FIRST_PERSON) {
        // 一人称視点の場合はカメラ位置・向きに追従
        lightManager_->UpdateSpotLightForFirstPerson(camera_);
    } else {
        // それ以外はプレイヤーに追従
        lightManager_->UpdateSpotLightPosition(player_->GetPosition(), player_->GetRotationY());
    }

    // ライトの設定をオブジェクトに適用
    const DirectionalLight& dirLight = lightManager_->GetDirectionalLight();
    const SpotLight& spotLight = lightManager_->GetSpotLight();

    player_->SetDirectionalLight(dirLight);
    player_->SetSpotLight(spotLight);
    
    ground_->SetDirectionalLight(dirLight);
    ground_->SetSpotLight(spotLight);

    // マルチマテリアルオブジェクトのUpdate
    for (auto& object : cubeGlbObjects_) {
        if (object) {
            object->SetDirectionalLight(dirLight);
            object->SetSpotLight(spotLight);
            object->Update();
        }
    }

    // Blender JSONオブジェクトのUpdate
    engine_->UpdateBlenderScene(dirLight, spotLight);

    // オブジェクトの更新
    player_->Update(engine_);
    ground_->Update();
}

void GamePlayScene::Draw() {
#ifdef _DEBUG
    BuildImGuiWindows();
#endif

    // オフスクリーンレンダリング開始
    engine_->BeginOffscreenRendering();

    // スカイボックスの描画
    if (skyboxEnabled_) {
        engine_->DrawSkybox();
    }
 
    spriteCommon_->CommonDraw();

    // オブジェクトの描画
    ground_->Draw();
    
    // マルチマテリアルオブジェクトのDraw
    for (auto& object : cubeGlbObjects_) {
        if (object) {
            object->Draw();
        }
    }

    // Blender JSONオブジェクトのDraw
    engine_->DrawBlenderScene();

    player_->Draw();

    // オフスクリーンレンダリング終了
    engine_->EndOffscreenRendering();
}

#ifdef _DEBUG
void GamePlayScene::BuildImGuiWindows() {
    // ImGuiの初期化状態をチェック
    if (!ImGui::GetCurrentContext()) {
        return;
    }
    
    // ライティング設定の描画
    lightManager_->DrawImGui();
    
    // ポストプロセス効果選択UI
    if (ImGui::Begin("Post-Processing Effects")) {
        ImGui::Text("Select Rendering Mode:");
        ImGui::Separator();
        
        if (ImGui::RadioButton("Normal", currentPostProcessMode_ == 0)) {
            currentPostProcessMode_ = 0;
            engine_->SetPostProcessMode(0);
        }
        
        if (ImGui::RadioButton("Grayscale", currentPostProcessMode_ == 1)) {
            currentPostProcessMode_ = 1;
            engine_->SetPostProcessMode(1);
        }
        
        if (ImGui::RadioButton("Vignetting", currentPostProcessMode_ == 2)) {
            currentPostProcessMode_ = 2;
            engine_->SetPostProcessMode(2);
        }
        
        if (ImGui::RadioButton("Horror", currentPostProcessMode_ == 3)) {
            currentPostProcessMode_ = 3;
            engine_->SetPostProcessMode(3);
        }
        
        ImGui::Separator();
        ImGui::Text("Keyboard shortcuts:");
        ImGui::Text("1 - Normal mode");
        ImGui::Text("2 - Grayscale mode");
        ImGui::Text("3 - Vignetting mode");
        ImGui::Text("4 - Horror mode");
    }
    ImGui::End();
}
#else
void GamePlayScene::BuildImGuiWindows() {
    // Releaseビルドでは何もしない
}
#endif

void GamePlayScene::Finalize() {
    if (player_) {
        player_->Finalize();
        player_.reset();
    }

    if (ground_) {
        ground_->Finalize();
        ground_.reset();
    }

    if (lightManager_) {
        lightManager_.reset();
    }

    // ポストプロセスを無効化
    engine_->EnablePostProcessing(false);

    // オブジェクトのクリーンアップ
    cubeGlbObjects_.clear();
}