#include "GamePlayScene.h"
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

    camera_->SetFov(1.37f);
    
    // 右スティック感度の調整（必要に応じて）
    engine_->SetCameraStickSensitivity(2.5f);
    
    // オービットカメラの初期設定
    engine_->SetCameraOrbitDistance(3.0f);
    engine_->SetCameraOrbitHeight(2.5f);
    
    // カメラの初期ターゲットを設定（プレイヤーの位置）
    engine_->SetCameraOrbitTarget(Vector3{0.0f, 0.0f, 0.0f});

    // マネージャーの初期化
    lightManager_ = std::make_unique<LightManager>();
    lightManager_->Initialize();

    // プレイヤーの作成と初期化
    player_ = std::make_unique<Player>();
    player_->Initialize(camera_);
    
    // グラウンドの作成と初期化
    ground_ = std::make_unique<Ground>();
    ground_->Initialize(camera_, dxCommon_);
    
    // Skyboxの作成と初期化（全ビルド設定で有効）
    skybox_ = engine_->CreateSkybox();
    skybox_->SetScale(1000.0f); // 大きなスケールで遠景を表現
    
    // DDS読み込みと描画を実行
    try {
        engine_->LoadSkybox(skybox_.get(), "Resources/Models/skybox/rostock_laage_airport_4k.dds");
        skyboxEnabled_ = true;
        OutputDebugStringA("Skybox: Successfully loaded with DDS texture\n");
    } catch (const std::exception& e) {
        skyboxEnabled_ = false;
        OutputDebugStringA(("Skybox: Failed to load DDS texture: " + std::string(e.what()) + "\n").c_str());
    }
    
    // 環境マップを自動適用（強制的に適用）
    engine_->SetEnvMap(player_);
    engine_->SetEnvMap(ground_);
    OutputDebugStringA("GamePlayScene: Environment map applied to player and ground\n");
    
    // Playerにさらに強制的に環境マップを有効化
    player_->SetEnableEnvironmentMap(true);
    player_->SetEnvironmentTexture("Resources/Models/skybox/rostock_laage_airport_4k.dds");
    OutputDebugStringA("GamePlayScene: Forced environment map enabled for player\n");
    
    if (skyboxEnabled_) {
        OutputDebugStringA("GamePlayScene: Skybox is enabled\n");
    } else {
        OutputDebugStringA("GamePlayScene: Skybox is disabled but environment map still applied\n");
    }
    
    // GLBキューブモデルの初期化（マルチマテリアル対応）
    try {
        // マルチマテリアルGLBファイルを読み込み
        Model tempModel;
        tempModel.Initialize(dxCommon_);
        std::vector<ModelData> multiMaterialData = tempModel.LoadMultiMaterialGLB("Resources/Models/cube/obje.glb");
        
        OutputDebugStringA(("GLB Multi-material: Loaded " + std::to_string(multiMaterialData.size()) + " materials\n").c_str());
        
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
            object3d->SetPosition(Vector3{ 2.0f, 0.0f, 0.0f });
            object3d->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
            object3d->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
            object3d->SetEnableLighting(true);
            object3d->SetCamera(camera_);
            
            // 環境マップを自動適用（Skybox有効時のみ）
            if (skyboxEnabled_) {
                engine_->SetEnvMap(object3d.get());
            }
            
            // ベクターに追加
            cubeGlbModels_.push_back(std::move(model));
            cubeGlbObjects_.push_back(std::move(object3d));
            
            OutputDebugStringA(("GLB Material [" + std::to_string(i) + "] loaded successfully\n").c_str());
        }
        
        if (!cubeGlbModels_.empty()) {
            OutputDebugStringA(("GLB Multi-material cube model loaded successfully with " + std::to_string(cubeGlbModels_.size()) + " materials\n").c_str());
        }
    } catch (const std::exception& e) {
        OutputDebugStringA(("Failed to load GLB multi-material cube model: " + std::string(e.what()) + "\n").c_str());
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

    // オブジェクトの更新
    if (skyboxEnabled_ && skybox_) {
        skybox_->Update();
    }
    player_->Update(engine_);
    ground_->Update();
}

void GamePlayScene::Draw() {
    if (!initialized_) return;

    spriteCommon_->CommonDraw();

    // Skyboxの描画（最初に背景として描画、有効な場合のみ）
    if (skyboxEnabled_ && skybox_) {
        skybox_->Draw(camera_);
        
        // Skyboxが独自のルートシグネチャを使用するため、再度CommonDrawを呼び出してルートシグネチャを復元
        spriteCommon_->CommonDraw();
    }

    // オブジェクトの描画
    ground_->Draw();
    
    // マルチマテリアルオブジェクトのDraw
    for (auto& object : cubeGlbObjects_) {
        if (object) {
            object->Draw();
        }
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
    ImGui::Text("1 / 右Shift - アニメーション切り替え（SneakWalk ⇔ Walk）");
    ImGui::Text("矢印キー - Humanモデル移動");
    ImGui::Text("F - ライティング設定の表示/非表示");
    ImGui::Text("ESC - 終了");
#ifdef _DEBUG
    ImGui::Text("F1 - フリーカメラモード切り替え");
#endif
    
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
    ImGui::Text("カメラモード: %s", camera_->IsFreeCameraMode() ? "フリーカメラ" : "プレイヤー追従");
    ImGui::Text("F1キーで切り替え");
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
    
    // 環境マップ設定
    ImGui::Separator();
    ImGui::Text("環境マップ設定:");
    
    // プレイヤーの環境マップ設定（強化版）
    ImGui::Separator();
    ImGui::Text("=== Human Model Environment Map ===");
    bool playerEnvMap = player_->GetEnableEnvironmentMap();
    if (ImGui::Checkbox("プレイヤー環境マップ (Human)", &playerEnvMap)) {
        player_->SetEnableEnvironmentMap(playerEnvMap);
        OutputDebugStringA(playerEnvMap ? "GamePlayScene: Player environment map ENABLED\n" : "GamePlayScene: Player environment map DISABLED\n");
    }
    ImGui::SameLine();
    ImGui::Text(playerEnvMap ? "[ON]" : "[OFF]");
    
    // 強制有効化ボタン
    if (ImGui::Button("Force Enable Human EnvMap")) {
        player_->SetEnableEnvironmentMap(true);
        player_->SetEnvironmentTexture("Resources/Models/skybox/rostock_laage_airport_4k.dds");
        OutputDebugStringA("GamePlayScene: FORCED Human environment map enabled!\n");
    }
    
    // グラウンドの環境マップ設定
    bool groundEnvMap = ground_->GetEnableEnvironmentMap();
    if (ImGui::Checkbox("グラウンド環境マップ", &groundEnvMap)) {
        ground_->SetEnableEnvironmentMap(groundEnvMap);
    }
    
    // マルチマテリアルキューブの環境マップ設定
    if (!cubeGlbObjects_.empty()) {
        bool cubeEnvMap = cubeGlbObjects_[0]->GetEnableEnvironmentMap();
        if (ImGui::Checkbox("キューブ環境マップ", &cubeEnvMap)) {
            for (auto& object : cubeGlbObjects_) {
                if (object) {
                    object->SetEnableEnvironmentMap(cubeEnvMap);
                }
            }
        }
    }

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

    // Skybox関連のクリーンアップ（有効だった場合のみ）
    if (skyboxEnabled_ && skybox_) {
        skybox_.reset();
    }
    skyboxEnabled_ = false;

    // マルチマテリアルオブジェクトのクリーンアップ
    cubeGlbObjects_.clear();
    cubeGlbModels_.clear();
}