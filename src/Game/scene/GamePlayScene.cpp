#include "GamePlayScene.h"
#ifdef _DEBUG
#include "imgui.h"
#include "imgui_impl_dx12.h"
#endif
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
    } catch (const std::exception&) {
        skyboxEnabled_ = false;
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
    } catch (const std::exception&) {
        // エラー処理
    }

    // Blender JSONシーンの読み込み
    blenderJsonLoader_ = std::make_unique<BlenderJSONLoader>();
    if (blenderJsonLoader_->LoadScene("Resources/blender/obje.json", dxCommon_, spriteCommon_)) {
        // ロードされたメッシュにカメラとライティングを設定
        for (auto& mesh : blenderJsonLoader_->GetLoadedMeshes()) {
            mesh.object3d->SetCamera(camera_);
            mesh.object3d->SetEnableLighting(true);
        }
    } else {
        // エラー処理
    }

    initialized_ = true;
}

void GamePlayScene::Update() {
    if (!initialized_) return;

    // デルタタイムを取得
    float deltaTime = engine_->GetDeltaTime();

    // ESCキーで終了
    if (engine_->IsPause()) {
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

    // フィルター効果切り替え
    if (engine_->IsKeyTriggered(DIK_1)) {
        engine_->SetPostProcessMode(0); // Normal
    }
    if (engine_->IsKeyTriggered(DIK_2)) {
        engine_->SetPostProcessMode(1); // Grayscale
    }
    if (engine_->IsKeyTriggered(DIK_3)) {
        engine_->SetPostProcessMode(2); // Vignetting
    }

    // カメラ入力処理（マウス + 右スティックを統合）
    engine_->UpdateCameraInput();
    
#ifdef _DEBUG
    // フリーカメラモードでない場合のみプレイヤー追従
    if (!engine_->IsFreeCameraMode()) {
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
    // フリーカメラモードの移動処理
    engine_->UpdateFreeCameraMovement();
    
    if (!engine_->IsFreeCameraMode()) {
#endif
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
#ifdef _DEBUG
    BuildImGuiWindows();
#endif

    // オフスクリーンレンダリング開始
    engine_->BeginOffscreenRendering();

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
        
        static int currentMode = 0;
        
        if (ImGui::RadioButton("Normal", currentMode == 0)) {
            currentMode = 0;
            engine_->SetPostProcessMode(0);
        }
        
        if (ImGui::RadioButton("Grayscale", currentMode == 1)) {
            currentMode = 1;
            engine_->SetPostProcessMode(1);
        }
        
        if (ImGui::RadioButton("Vignetting", currentMode == 2)) {
            currentMode = 2;
            engine_->SetPostProcessMode(2);
        }
        
        ImGui::Separator();
        ImGui::Text("Keyboard shortcuts:");
        ImGui::Text("1 - Normal mode");
        ImGui::Text("2 - Grayscale mode");
        ImGui::Text("3 - Vignetting mode");
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

    // Skybox関連のクリーンアップ（有効だった場合のみ）
    if (skyboxEnabled_ && skybox_) {
        skybox_.reset();
    }
    skyboxEnabled_ = false;

    // マルチマテリアルオブジェクトのクリーンアップ
    cubeGlbObjects_.clear();
    cubeGlbModels_.clear();
}