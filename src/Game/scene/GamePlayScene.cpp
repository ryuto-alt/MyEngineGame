#include "GamePlayScene.h"


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

    camera_->SetTranslate(Vector3{ 0.0f, 0.0f, -2.0f });
    camera_->SetFovY(1.37f);
    
    // カメラにウィンドウハンドルを設定
    camera_->SetWindowHandle(engine_->GetWinApp()->GetHwnd());

    // マネージャーの初期化
    lightManager_ = std::make_unique<LightManager>();
    lightManager_->Initialize();

    // プレイヤーの作成と初期化
    player_ = std::make_unique<Player>();
    player_->Initialize(camera_);
    
    // グラウンドの作成と初期化
    ground_ = std::make_unique<Ground>();
    ground_->Initialize(camera_, dxCommon_);
    
    // 環境マップテクスチャを設定（Human/Playerモデルに環境マップを適用）
    player_->SetEnvironmentTexture("Resources/Models/skybox/rostock_laage_airport_4k.dds");
    ground_->SetEnvironmentTexture("Resources/Models/skybox/rostock_laage_airport_4k.dds");
    
    // Skyboxの作成と初期化
    skybox_ = std::make_unique<Skybox>();
    skybox_->Initialize(dxCommon_, engine_->GetSrvManager(), engine_->GetTextureManager());
    // 正しいパスのDDSファイルを読み込み
    skybox_->LoadCubemap("Resources/Models/skybox/rostock_laage_airport_4k.dds");
    skybox_->SetScale(1000.0f); // 大きなスケールで遠景を表現
    
    // 環境マップテクスチャを事前にロードして描画中の動的SRV作成を避ける
    engine_->GetTextureManager()->LoadTexture("Resources/Models/skybox/rostock_laage_airport_4k.dds");
    
    // GLBキューブモデルの初期化
    try {
        cubeGlbModel_ = std::make_unique<Model>();
        cubeGlbModel_->Initialize(dxCommon_);
        cubeGlbModel_->LoadFromGLB("Resources/Models/cube/obje.glb");

        if (cubeGlbModel_) {
            cubeGlbObject3d_ = engine_->CreateObject3D();
            cubeGlbObject3d_->SetModel(cubeGlbModel_.get());
            cubeGlbObject3d_->SetPosition(Vector3{ 2.0f, 0.0f, 0.0f });
            cubeGlbObject3d_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
            cubeGlbObject3d_->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
            cubeGlbObject3d_->SetEnableLighting(true);
            cubeGlbObject3d_->SetCamera(camera_);
            
            // 環境マップテクスチャを設定（skyboxと同じキューブマップを使用）
            cubeGlbObject3d_->SetEnvironmentTexture("Resources/Models/skybox/rostock_laage_airport_4k.dds");
            
            OutputDebugStringA("GLB Cube model loaded successfully\n");
        }
    } catch (const std::exception& e) {
        OutputDebugStringA(("Failed to load GLB cube model: " + std::string(e.what()) + "\n").c_str());
    }

    initialized_ = true;
}

void GamePlayScene::Update() {
    if (!initialized_) return;

    // デルタタイムを取得
    float deltaTime = engine_->GetDeltaTime();

    // ESCキーで終了
    if (engine_->IsKeyTriggered(DIK_ESCAPE)) {
        exit(0);
    }

#ifdef _DEBUG
    // TABキーでマウス視点移動の切り替え
    if (engine_->IsKeyTriggered(DIK_TAB)) {
        camera_->ToggleMouseLook();
    }
#endif

    // マウス入力処理
    float deltaX, deltaY;
    engine_->GetInput()->GetMouseMovement(deltaX, deltaY);
    camera_->ProcessMouseInput(deltaX, deltaY);

    // カメラ移動（カメラの向きに基づく移動、デルタタイム考慮）
    float cameraMoveSpeed = cameraSpeed_ * deltaTime;
    if (engine_->IsKeyPressed(DIK_W)) camera_->MoveForward(cameraMoveSpeed);    // 前方移動
    if (engine_->IsKeyPressed(DIK_S)) camera_->MoveForward(-cameraMoveSpeed);   // 後方移動
    if (engine_->IsKeyPressed(DIK_A)) camera_->MoveRight(-cameraMoveSpeed);     // 左移動
    if (engine_->IsKeyPressed(DIK_D)) camera_->MoveRight(cameraMoveSpeed);      // 右移動
    if (engine_->IsKeyPressed(DIK_SPACE)) camera_->MoveUp(cameraMoveSpeed);     // 上昇
    if (engine_->IsKeyPressed(DIK_LSHIFT)) camera_->MoveUp(-cameraMoveSpeed);   // 下降

    // Fキーでライティングデバッグウィンドウの表示切り替え
    if (engine_->IsKeyTriggered(DIK_F)) {
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
    if (engine_->IsKeyTriggered(DIK_1)) {
        player_->ToggleSneakWalk();
    }

    // ライトマネージャーの更新
    lightManager_->Update();

    // ライトの設定をオブジェクトに適用
    const DirectionalLight& dirLight = lightManager_->GetDirectionalLight();
    const SpotLight& spotLight = lightManager_->GetSpotLight();

    player_->SetDirectionalLight(dirLight);
    player_->SetSpotLight(spotLight);
    
    ground_->SetDirectionalLight(dirLight);
    ground_->SetSpotLight(spotLight);

    if (cubeGlbObject3d_) {
        cubeGlbObject3d_->SetDirectionalLight(dirLight);
        cubeGlbObject3d_->SetSpotLight(spotLight);
        cubeGlbObject3d_->Update();
    }

    // オブジェクトの更新
    skybox_->Update();
    player_->Update(engine_);
    ground_->Update();

    camera_->Update();
}

void GamePlayScene::Draw() {
    if (!initialized_) return;

    spriteCommon_->CommonDraw();

    // Skyboxの描画（最初に背景として描画）
    skybox_->Draw(camera_);
    
    // Skyboxが独自のルートシグネチャを使用するため、再度CommonDrawを呼び出してルートシグネチャを復元
    spriteCommon_->CommonDraw();

    // オブジェクトの描画
    ground_->Draw();
    
    if (cubeGlbObject3d_) {
        cubeGlbObject3d_->Draw();
    }
    
    player_->Draw();

    // ImGUI描画
#pragma region imgui
    ImGui::Begin("Human Animation Demo ");

    ImGui::Text("操作方法:");
    ImGui::Separator();
    ImGui::Text("【キーボード】");
    ImGui::Text("WASD - カメラ移動");
    ImGui::Text("SPACE - 上昇");
    ImGui::Text("SHIFT - 下降");
    ImGui::Text("P - アニメーション一時停止/再開");
    ImGui::Text("R - アニメーションリセット");
    ImGui::Text("1 - アニメーション切り替え（SneakWalk ⇔ Walk）");
    ImGui::Text("F - ライティング設定の表示/非表示");
    ImGui::Text("ESC - 終了");
    
    ImGui::Separator();
    ImGui::Text("【Xboxコントローラー】");
    ImGui::Text("左スティック - ヒューマンモデル移動");
    ImGui::Text("移動中にBボタン - スニーク状態");
    ImGui::Text("※停止中はスニーク無効");
    ImGui::Text("※コントローラー接続: %s", engine_->IsXboxControllerConnected() ? "接続済み" : "未接続");

    ImGui::Separator();
    Vector3 cameraPos = camera_->GetTranslate();
    Vector3 cameraRot = camera_->GetRotate();
    ImGui::Text("カメラ位置: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::Text("カメラ回転: (%.2f, %.2f, %.2f)", cameraRot.x, cameraRot.y, cameraRot.z);
    
#ifdef _DEBUG
    ImGui::Text("マウス視点移動: %s", camera_->IsMouseLookEnabled() ? "ON" : "OFF");
    ImGui::Text("TABキーで切り替え");
#endif
    
    // アニメーション情報
    ImGui::Separator();
    ImGui::Text("現在のアニメーション: %s", player_->GetCurrentAnimationName().c_str());
    
    std::string stateText;
    if (player_->IsBlending()) {
        stateText = "ブレンド中";
    } else if (player_->IsSneaking()) {
        stateText = "スニーク中";
    } else if (player_->IsMoving()) {
        stateText = "移動中";
    } else {
        stateText = "停止中";
    }
    ImGui::Text("状態: %s", stateText.c_str());
    
    if (player_->IsBlending()) {
        ImGui::Text("ブレンド進行: %.1f%%", player_->GetBlendProgress() * 100.0f);
    }

    // 回転スムーシング設定
    ImGui::Separator();
    ImGui::Text("回転スムーシング設定:");
    float smoothingSpeed = player_->GetRotationSmoothingSpeed();
    if (ImGui::SliderFloat("スムーシング速度", &smoothingSpeed, 0.1f, 20.0f, "%.1f")) {
        player_->SetRotationSmoothingSpeed(smoothingSpeed);
    }
    ImGui::Text("現在の回転: %.2f度", player_->GetRotationY() * 180.0f / 3.14159f);
    ImGui::Text("目標回転: %.2f度", player_->GetTargetRotationY() * 180.0f / 3.14159f);

    ImGui::End();
    
    // ライティング設定の描画
    lightManager_->DrawImGui();
#pragma endregion
}

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

    if (cubeGlbObject3d_) {
        cubeGlbObject3d_.reset();
    }

    if (cubeGlbModel_) {
        cubeGlbModel_.reset();
    }
}