#include "GamePlayScene.h"
#include "OffscreenRenderingManager.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
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

    // オフスクリーンレンダリングマネージャーの初期化
    offscreenRenderingManager_ = std::make_unique<OffscreenRenderingManager>();
    Vector4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // 黒色の背景
    offscreenRenderingManager_->Initialize(
        engine_->GetDirectXCommon(),
        engine_->GetSrvManager(),
        1280, // WinApp::kClientWidth
        720,  // WinApp::kClientHeight
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        clearColor
    );
    
    // 初期状態はNormalモードに設定
    offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Normal);

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
    
    // マネージャーの初期化
    lightManager_ = std::make_unique<LightManager>();
    lightManager_->Initialize();

    // プレイヤーの作成と初期化
    player_ = std::make_unique<Player>();
    player_->Initialize(camera_);
    
    // グラウンドの作成と初期化
    ground_ = std::make_unique<Ground>();
    ground_->Initialize(camera_, dxCommon_);
    
    // スカイボックスの作成と初期化
    skybox_ = engine_->CreateSkybox();
    skybox_->SetScale(1000.0f);
    try {
        engine_->LoadSkybox(skybox_.get(), "Resources/Models/skybox/rostock_laage_airport_4k.dds");
        skyboxEnabled_ = true;
        OutputDebugStringA("Skybox loaded successfully\n");
    } catch (const std::exception& e) {
        skyboxEnabled_ = false;
        std::string errorMsg = "Failed to load skybox: " + std::string(e.what()) + "\n";
        OutputDebugStringA(errorMsg.c_str());
    }
    
    // オフスクリーンレンダリング専用モードで動作
    
    // GLBキューブモデルの初期化（マルチマテリアル対応）
    try {
        // マルチマテリアルGLBファイルを読み込み
        Model tempModel;
        tempModel.Initialize(dxCommon_);
        std::vector<ModelData> multiMaterialData = tempModel.LoadMultiMaterialGLB("Resources/Models/cube/obje.glb");
        
        // GLBマルチマテリアル読み込み完了
        
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
            object3d->SetScale(Vector3{ 1.0f, 1.0f, 1.0f }); // 大きくして見えやすく
            object3d->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
            object3d->SetEnableLighting(true);
            object3d->SetCamera(camera_);
            
            // 環境マップは無効（skyboxが無効のため）
            
            // ベクターに追加
            cubeGlbModels_.push_back(std::move(model));
            cubeGlbObjects_.push_back(std::move(object3d));
            
            // マテリアル読み込み完了
        }
        
        // GLBモデル読み込み完了
    } catch (const std::exception& e) {
        std::string errorMsg = "Failed to load GLB multi-material cube model: " + std::string(e.what()) + "\n";
        OutputDebugStringA(errorMsg.c_str());
    }

    // Blender JSONシーンの読み込み
    blenderJsonLoader_ = std::make_unique<BlenderJSONLoader>();
    if (blenderJsonLoader_->LoadScene("Resources/blender/obje.json", dxCommon_, spriteCommon_)) {
        // ロードされたメッシュにカメラとライティングを設定
        for (auto& mesh : blenderJsonLoader_->GetLoadedMeshes()) {
            mesh.object3d->SetCamera(camera_);
            mesh.object3d->SetEnableLighting(true);
        }
        OutputDebugStringA("Blender JSON scene loaded successfully\n");
    } else {
        std::string errorMsg = "Failed to load Blender JSON scene: " + blenderJsonLoader_->GetErrorMessage() + "\n";
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
    // キーボードでの簡単な切り替えも残しておく
    static bool key1Pressed = false, key2Pressed = false, key3Pressed = false;
    
    if (engine_->IsKeyPressed(DIK_1) && !key1Pressed) {
        offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Normal);
        OutputDebugStringA("GamePlayScene: Switched to Normal mode via keyboard\n");
        key1Pressed = true;
    } else if (!engine_->IsKeyPressed(DIK_1)) {
        key1Pressed = false;
    }
    
    if (engine_->IsKeyPressed(DIK_2) && !key2Pressed) {
        offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Grayscale);
        OutputDebugStringA("GamePlayScene: Switched to Grayscale mode via keyboard\n");
        key2Pressed = true;
    } else if (!engine_->IsKeyPressed(DIK_2)) {
        key2Pressed = false;
    }
    
    if (engine_->IsKeyPressed(DIK_3) && !key3Pressed) {
        offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Vignetting);
        OutputDebugStringA("GamePlayScene: Switched to Vignetting mode via keyboard\n");
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

    // Blender JSONオブジェクトのUpdate
    if (blenderJsonLoader_) {
        for (const auto& mesh : blenderJsonLoader_->GetLoadedMeshes()) {
            if (mesh.object3d) {
                mesh.object3d->SetDirectionalLight(dirLight);
                mesh.object3d->SetSpotLight(spotLight);
                mesh.object3d->Update();
            }
        }
    }

    // オブジェクトの更新
    player_->Update(engine_);
    ground_->Update();
}

void GamePlayScene::Draw() {

    BuildImGuiWindows();

    // オフスクリーンレンダリング開始
    offscreenRenderingManager_->BeginRenderToTexture();

    // スカイボックスの描画（有効な場合）
    if (skyboxEnabled_ && skybox_) {
        skybox_->Draw(camera_);
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
    if (blenderJsonLoader_) {
        for (const auto& mesh : blenderJsonLoader_->GetLoadedMeshes()) {
            if (mesh.object3d) {
                mesh.object3d->Draw();
            }
        }
    }

    player_->Draw();

    // オフスクリーンレンダリング終了
    offscreenRenderingManager_->EndRenderToTexture();

    // SwapChainに画面をコピー
    offscreenRenderingManager_->CopyToSwapChain();
}

void GamePlayScene::BuildImGuiWindows() {
    // ImGuiの初期化状態をチェック
    if (!ImGui::GetCurrentContext()) {
        return;
    }
    
#pragma region imgui
  
    
    // ライティング設定の描画
    lightManager_->DrawImGui();
    
    // ポストプロセス効果選択UI
    if (ImGui::Begin("Post-Processing Effects")) {
        ImGui::Text("Select Rendering Mode:");
        ImGui::Separator();
        
        ProcessingMode currentMode = offscreenRenderingManager_->GetProcessingMode();
        
        if (ImGui::RadioButton("Normal", currentMode == ProcessingMode::Normal)) {
            offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Normal);
            OutputDebugStringA("GamePlayScene: Switched to Normal mode via ImGui\n");
        }
        
        if (ImGui::RadioButton("Grayscale", currentMode == ProcessingMode::Grayscale)) {
            offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Grayscale);
            OutputDebugStringA("GamePlayScene: Switched to Grayscale mode via ImGui\n");
        }
        
        if (ImGui::RadioButton("Vignetting", currentMode == ProcessingMode::Vignetting)) {
            offscreenRenderingManager_->SetProcessingMode(ProcessingMode::Vignetting);
            OutputDebugStringA("GamePlayScene: Switched to Vignetting mode via ImGui\n");
        }
        
        ImGui::Separator();
        ImGui::Text("Keyboard shortcuts:");
        ImGui::Text("1 - Normal mode");
        ImGui::Text("2 - Grayscale mode");
        ImGui::Text("3 - Vignetting mode");
    }
    ImGui::End();
    
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