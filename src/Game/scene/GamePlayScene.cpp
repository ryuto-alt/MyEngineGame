#include "GamePlayScene.h"
#include "OffscreenRenderingManager.h"
#include <string>
#include <vector>
#include <memory>


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

    // オフスクリーンレンダリングマネージャーの初期化（テスト用に赤色の背景で初期化）
    offscreenRenderingManager_ = std::make_unique<OffscreenRenderingManager>();
    Vector4 redClearColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // 【テスト用】赤色でオフスクリーンレンダリング確認
    offscreenRenderingManager_->Initialize(
        engine_->GetDirectXCommon(),
        engine_->GetSrvManager(),
        1280, // WinApp::kClientWidth
        720,  // WinApp::kClientHeight
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        redClearColor
    );
    
    // 【テスト用】初期状態で強制的にグレースケールモードに設定
    OutputDebugStringA("GamePlayScene::Initialize - *** FORCING GRAYSCALE MODE FOR TESTING ***\n");
    offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Grayscale);
    OutputDebugStringA("GamePlayScene::Initialize - Grayscale mode set at initialization\n");
    OutputDebugStringA("GamePlayScene::Initialize - Press 1=Normal, 2=Grayscale, 3=Sepia to switch modes\n");

    camera_->SetFov(1.37f);
    
    // 【重要修正】カメラを3Dオブジェクトが見える位置に明示的に配置
    camera_->SetTranslate(Vector3{ 0.0f, 2.0f, -10.0f }); // プレイヤーの後ろ上方から見下ろす
    camera_->SetRotate(Vector3{ 0.1f, 0.0f, 0.0f }); // 少し下向き
    
    // 右スティック感度の調整（必要に応じて）
    engine_->SetCameraStickSensitivity(2.5f);
    
    // オービットカメラの初期設定
    engine_->SetCameraOrbitDistance(8.0f); // 距離を広げて全体が見えるように
    engine_->SetCameraOrbitHeight(3.0f);   // 高さを上げて俯瞰視点に
    
    // カメラの初期ターゲットを設定（プレイヤーの位置）
    engine_->SetCameraOrbitTarget(Vector3{0.0f, 0.0f, 0.0f});

    // 【修正】オフスクリーンレンダリングと3Dモデル描画を両方有効化
    // Skyboxは無効のまま（ddsファイルは使わない）
    
    // マネージャーの初期化
    lightManager_ = std::make_unique<LightManager>();
    lightManager_->Initialize();

    // プレイヤーの作成と初期化
    player_ = std::make_unique<Player>();
    player_->Initialize(camera_);
    
    // グラウンドの作成と初期化
    ground_ = std::make_unique<Ground>();
    ground_->Initialize(camera_, dxCommon_);
    
    // Skybox関連は無効のまま（ddsファイルは使わない）
    /*
    skybox_ = engine_->CreateSkybox();
    skybox_->SetScale(1000.0f);
    try {
        engine_->LoadSkybox(skybox_.get(), "Resources/Models/skybox/rostock_laage_airport_4k.dds");
        skyboxEnabled_ = true;
    } catch (const std::exception& e) {
        skyboxEnabled_ = false;
    }
    */
    skyboxEnabled_ = false;
    
    OutputDebugStringA("GamePlayScene: 3D models initialized (Player, Ground, LightManager)\n");
    
    // オフスクリーンレンダリング専用モードで動作
    OutputDebugStringA("GamePlayScene: Running in Offscreen Rendering Only Mode\n");
    
    // GLBキューブモデルの初期化（マルチマテリアル対応）
    try {
        // マルチマテリアルGLBファイルを読み込み
        Model tempModel;
        tempModel.Initialize(dxCommon_);
        std::vector<ModelData> multiMaterialData = tempModel.LoadMultiMaterialGLB("Resources/Models/cube/obje.glb");
        
        std::string matCountMsg = "GLB Multi-material: Loaded " + std::to_string(multiMaterialData.size()) + " materials\n";
        OutputDebugStringA(matCountMsg.c_str());
        
        // 詳細なマテリアル情報を出力
        for (size_t i = 0; i < multiMaterialData.size(); ++i) {
            char debugMsg[512];
            sprintf_s(debugMsg, "Material[%zu]: BaseColor=(%.3f,%.3f,%.3f,%.3f), Metallic=%.3f, Roughness=%.3f, Vertices=%zu, isPBR=%s\n",
                i, multiMaterialData[i].material.baseColorFactor.x, multiMaterialData[i].material.baseColorFactor.y,
                multiMaterialData[i].material.baseColorFactor.z, multiMaterialData[i].material.baseColorFactor.w,
                multiMaterialData[i].material.metallicFactor, multiMaterialData[i].material.roughnessFactor,
                multiMaterialData[i].vertices.size(), multiMaterialData[i].material.isPBR ? "true" : "false");
            OutputDebugStringA(debugMsg);
        }
        
        // 各マテリアルごとに別々のModelとObject3dを作成
        for (size_t i = 0; i < multiMaterialData.size(); ++i) {
            // Modelを作成
            auto model = std::make_unique<Model>();
            model->Initialize(dxCommon_);
            
            // ModelDataを手動で設定
            ModelData& modelData = model->GetModelDataInternal();
            modelData = multiMaterialData[i];
            model->CreateVertexBuffer();
            
            // テクスチャを読み込み
            if (!modelData.material.textureFilePath.empty()) {
                TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
            }
            
            // Object3dを作成
            auto object3d = engine_->CreateObject3D();
            object3d->SetModel(model.get());
            object3d->SetPosition(Vector3{ 3.0f, 1.0f, 2.0f }); // カメラから見える位置に配置
            object3d->SetScale(Vector3{ 2.0f, 2.0f, 2.0f }); // 大きくして見えやすく
            object3d->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
            object3d->SetEnableLighting(true);
            object3d->SetCamera(camera_);
            
            // 環境マップは無効（skyboxが無効のため）
            
            // ベクターに追加
            cubeGlbModels_.push_back(std::move(model));
            cubeGlbObjects_.push_back(std::move(object3d));
            
            std::string matLoadMsg = "GLB Material [" + std::to_string(i) + "] loaded successfully\n";
            OutputDebugStringA(matLoadMsg.c_str());
        }
        
        if (!cubeGlbModels_.empty()) {
            std::string successMsg = "GLB Multi-material cube model loaded successfully with " + std::to_string(cubeGlbModels_.size()) + " materials\n";
            OutputDebugStringA(successMsg.c_str());
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Failed to load GLB multi-material cube model: " + std::string(e.what()) + "\n";
        OutputDebugStringA(errorMsg.c_str());
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
    
    // F1キーでフリーカメラモード切り替え
    if (engine_->IsKeyTriggered(DIK_F1)) {
        camera_->ToggleFreeCameraMode();
    }
    
#endif

    // フィルター効果切り替えキーバインド（授業資料に基づく実装）
    // デバッグ用：キーの状態を常に監視（デバッグモード関係なく動作）
    static bool key1Pressed = false, key2Pressed = false, key3Pressed = false;
    
    if (engine_->IsKeyPressed(DIK_1) && !key1Pressed) {
        OutputDebugStringA("*** DEBUG: Key 1 pressed - Switching to Normal mode ***\n");
        offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Normal);
        OutputDebugStringA("GamePlayScene: Switched to Normal mode\n");
        key1Pressed = true;
    } else if (!engine_->IsKeyPressed(DIK_1)) {
        key1Pressed = false;
    }
    
    if (engine_->IsKeyPressed(DIK_2) && !key2Pressed) {
        OutputDebugStringA("*** DEBUG: Key 2 pressed - Switching to Grayscale mode ***\n");
        offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Grayscale);
        OutputDebugStringA("GamePlayScene: Switched to Grayscale mode\n");
        key2Pressed = true;
    } else if (!engine_->IsKeyPressed(DIK_2)) {
        key2Pressed = false;
    }
    
    if (engine_->IsKeyPressed(DIK_3) && !key3Pressed) {
        OutputDebugStringA("*** DEBUG: Key 3 pressed - Switching to Sepia mode ***\n");
        offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Sepia);
        OutputDebugStringA("GamePlayScene: Switched to Sepia mode\n");
        key3Pressed = true;
    } else if (!engine_->IsKeyPressed(DIK_3)) {
        key3Pressed = false;
    }

    // カメラ回転処理（マウス + 右スティック）
    engine_->UpdateCameraMouse();
    engine_->UpdateCameraRightStick();
    
#ifdef _DEBUG
    // フリーカメラモードでない場合のみプレイヤー追従
    if (!camera_->IsFreeCameraMode()) {
#endif
        // カメラフォロー更新（プレイヤー移動処理の前に実行）
        player_->UpdateCameraFollow();
#ifdef _DEBUG
    }
#endif
    
    // カメラの行列を即座に更新（プレイヤー移動で正しい方向ベクトルを取得するため）
    camera_->Update();

    // プレイヤー移動（カメラの向きに基づく統合移動、デルタタイム考慮）
#ifdef _DEBUG
    // フリーカメラモードの時はカメラを直接移動
    if (camera_->IsFreeCameraMode()) {
        float cameraSpeed = 5.0f * deltaTime;
        
        if (engine_->IsKeyPressed(DIK_W)) {
            camera_->MoveForward(cameraSpeed);
        }
        if (engine_->IsKeyPressed(DIK_S)) {
            camera_->MoveForward(-cameraSpeed);
        }
        if (engine_->IsKeyPressed(DIK_A)) {
            camera_->MoveRight(-cameraSpeed);
        }
        if (engine_->IsKeyPressed(DIK_D)) {
            camera_->MoveRight(cameraSpeed);
        }
        if (engine_->IsKeyPressed(DIK_SPACE)) {
            camera_->MoveUp(cameraSpeed);
        }
        if (engine_->IsKeyPressed(DIK_LSHIFT)) {
            camera_->MoveUp(-cameraSpeed);
        }
    } else {
#endif
        // 通常のプレイヤー移動処理
        float forward = 0.0f;
        float right = 0.0f;
        
        // キーボード入力（WASD）
        if (engine_->IsKeyPressed(DIK_W)) forward += 1.0f;   // 前方移動
        if (engine_->IsKeyPressed(DIK_S)) forward -= 1.0f;   // 後方移動
        if (engine_->IsKeyPressed(DIK_A)) right -= 1.0f;     // 左移動
        if (engine_->IsKeyPressed(DIK_D)) right += 1.0f;     // 右移動
        
        // ゲームパッド入力（左スティック）
        float stickX = engine_->GetXboxLeftStickX();
        float stickY = engine_->GetXboxLeftStickY();
        
        // デッドゾーン処理
        const float deadZone = 0.1f;
        if (abs(stickX) < deadZone) stickX = 0.0f;
        if (abs(stickY) < deadZone) stickY = 0.0f;
        
        // スティック入力を統合（Y軸は前後、X軸は左右）
        forward += stickY;
        right += stickX;
        
        // 入力値を正規化（最大値1.0にクランプ）
        float totalMagnitude = std::sqrt(forward * forward + right * right);
        if (totalMagnitude > 1.0f) {
            forward /= totalMagnitude;
            right /= totalMagnitude;
        }
        
        bool isPlayerMoving = (forward != 0.0f || right != 0.0f);
        
        if (isPlayerMoving) {
            player_->MoveWithCameraDirection(forward, right, deltaTime);
        }
        
        // 移動停止時の処理（WASDキーが押されていない場合）
        if (!isPlayerMoving && player_->IsMoving()) {
            player_->StopMoving();
        }
#ifdef _DEBUG
    }
#endif

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
    if (engine_->IsKeyTriggered(DIK_RSHIFT)) {
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

    // マルチマテリアルオブジェクトのUpdate
    for (auto& object : cubeGlbObjects_) {
        if (object) {
            object->SetDirectionalLight(dirLight);
            object->SetSpotLight(spotLight);
            object->Update();
        }
    }

    // オブジェクトの更新（skyboxは無効）
    player_->Update(engine_);
    ground_->Update();
}

void GamePlayScene::Draw() {
    if (!initialized_) {
        OutputDebugStringA("GamePlayScene::Draw - Not initialized, returning\n");
        return;
    }

    OutputDebugStringA("GamePlayScene::Draw - Starting draw process\n");
    
    if (!offscreenRenderingManager_) {
        OutputDebugStringA("GamePlayScene::Draw - ERROR: offscreenRenderingManager_ is null!\n");
        return;
    }

    OutputDebugStringA("GamePlayScene::Draw - About to call BeginRenderToTexture()\n");
    
    // オフスクリーンレンダリング開始（赤い背景でクリア）
    offscreenRenderingManager_->BeginRenderToTexture();

    OutputDebugStringA("GamePlayScene::Draw - BeginRenderToTexture() completed\n");

    // 【重要】オフスクリーンレンダリング内で3Dオブジェクトを描画
    spriteCommon_->CommonDraw();
    OutputDebugStringA("GamePlayScene::Draw - SpriteCommon CommonDraw completed\n");

    // Skyboxは無効（ddsファイルは使わない）

    // オブジェクトの描画
    OutputDebugStringA("GamePlayScene::Draw - About to draw Ground\n");
    ground_->Draw();
    OutputDebugStringA("GamePlayScene::Draw - Ground Draw completed\n");
    
    // マルチマテリアルオブジェクトのDraw
    OutputDebugStringA("GamePlayScene::Draw - About to draw GLB cube objects\n");
    for (auto& object : cubeGlbObjects_) {
        if (object) {
            OutputDebugStringA("GamePlayScene::Draw - Drawing GLB cube object\n");
            object->Draw();
        }
    }
    OutputDebugStringA("GamePlayScene::Draw - GLB cube objects Draw completed\n");
    
    OutputDebugStringA("GamePlayScene::Draw - About to draw Player\n");
    player_->Draw();
    OutputDebugStringA("GamePlayScene::Draw - Player Draw completed\n");

    OutputDebugStringA("GamePlayScene::Draw - About to call EndRenderToTexture()\n");

    // オフスクリーンレンダリング終了
    offscreenRenderingManager_->EndRenderToTexture();

    OutputDebugStringA("GamePlayScene::Draw - EndRenderToTexture() completed\n");
    OutputDebugStringA("GamePlayScene::Draw - About to call CopyToSwapChain()\n");

    // SwapChainに赤い画面をコピー
    offscreenRenderingManager_->CopyToSwapChain();

    OutputDebugStringA("GamePlayScene::Draw - CopyToSwapChain() completed\n");

    // ImGUI描画（オフスクリーンレンダリング後に通常のスワップチェインに描画）
#pragma region imgui
    ImGui::Begin("Human Animation Demo");

    ImGui::Text("操作方法:");
    ImGui::Separator();
    ImGui::Text("【キーボード】");
    ImGui::Text("WASD - プレイヤー移動");
    ImGui::Text("P - アニメーション一時停止/再開");
    ImGui::Text("R - アニメーションリセット");
    ImGui::Text("1 / 右Shift - アニメーション切り替え");
    ImGui::Text("F - ライティング設定の表示/非表示");
    ImGui::Text("ESC - 終了");
#ifdef _DEBUG
    ImGui::Text("F1 - フリーカメラモード切り替え");
    ImGui::Text("TAB - マウス視点移動切り替え");
#endif
    
    ImGui::Separator();
    ImGui::Text("【Xboxコントローラー】");
    ImGui::Text("左スティック - プレイヤー移動");
    ImGui::Text("右スティック - カメラ回転");
    ImGui::Text("※コントローラー接続: %s", engine_->IsXboxControllerConnected() ? "接続済み" : "未接続");

    ImGui::Separator();
    Vector3 cameraPos = camera_->GetTranslate();
    Vector3 cameraRot = camera_->GetRotate();
    ImGui::Text("カメラ位置: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::Text("カメラ回転: (%.2f, %.2f, %.2f)", cameraRot.x, cameraRot.y, cameraRot.z);
    
#ifdef _DEBUG
    ImGui::Text("マウス視点移動: %s", camera_->IsMouseLookEnabled() ? "ON" : "OFF");
    ImGui::Text("カメラモード: %s", camera_->IsFreeCameraMode() ? "フリーカメラ" : "プレイヤー追従");
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

    // オフスクリーンレンダリングマネージャーのクリーンアップ
    if (offscreenRenderingManager_) {
        offscreenRenderingManager_.reset();
    }

    // Skybox関連のクリーンアップ（有効だった場合のみ）
    if (skyboxEnabled_ && skybox_) {
        skybox_.reset();
    }
    skyboxEnabled_ = false;

    // マルチマテリアルオブジェクトのクリーンアップ
    cubeGlbObjects_.clear();
    cubeGlbModels_.clear();
}