#include "GamePlayScene.h"
#include "imgui.h"
#include "UnoEngine.h"
#include "SceneManager.h"
#include "AABBCollision.h"

void GamePlayScene::Initialize() {
    UnoEngine* engine = UnoEngine::GetInstance();

    lightManager_ = std::make_unique<LightManager>();
    lightManager_->Initialize();

    player_ = std::make_unique<Player>();
    player_->Initialize(camera_);
    player_->SetupCamera(engine);
    player_->SetEnableEnvironmentMap(false);

    ground_ = std::make_unique<Ground>();
    ground_->Initialize(camera_, dxCommon_);

    objeModel_ = engine->CreateAnimatedModel();
    objeModel_->LoadFromFile("Resources/Models/obje", "object.gltf");

    objeObject_ = engine->CreateObject3D();
    objeObject_->SetModel(static_cast<Model*>(objeModel_.get()));
    objeObject_->SetCamera(camera_);
    objeObject_->SetPosition({0.0f, 0.0f, 5.0f});
    objeObject_->SetScale({1.0f, 1.0f, 1.0f});
    objeObject_->SetEnableLighting(true);
    objeObject_->SetEnableAnimation(false);

    skyboxEnabled_ = false;

    // コリジョン設定
    SetupCollision();
}

void GamePlayScene::SetupCollision() {
    auto* collisionManager = Collision::AABBCollisionManager::GetInstance();
    if (!collisionManager) {
        return;
    }

    // Playerのコリジョン設定
    if (player_ && player_->GetObject() && player_->GetModel()) {
        Collision::AABB playerAABB = Collision::AABBExtractor::ExtractFromAnimatedModel(player_->GetModel());
        collisionManager->RegisterObject(player_->GetObject(), playerAABB, true, "Player");
    }

    // objeObjectのコリジョン設定（マルチメッシュ対応）
    if (objeObject_ && objeModel_) {
        std::vector<Collision::AABB> objeAABBs = Collision::AABBExtractor::ExtractMultipleAABBsFromAnimatedModel(objeModel_.get());
        collisionManager->RegisterObjectWithMultipleAABBs(objeObject_.get(), objeAABBs, true, "Object");
    }

    // Groundのコリジョン設定（一旦無効化）
    // if (ground_ && ground_->GetObject() && ground_->GetModel()) {
    //     Collision::AABB groundAABB = Collision::AABBExtractor::ExtractFromAnimatedModel(ground_->GetModel());
    //     collisionManager->RegisterObject(ground_->GetObject(), groundAABB, true, "Ground");
    // }
}

void GamePlayScene::Update() {
    UnoEngine* engine = UnoEngine::GetInstance();

    HandleInput();
    player_->HandleInput(engine);
    player_->UpdateCameraSystem(engine);

    lightManager_->Update();
    const DirectionalLight& dirLight = lightManager_->GetDirectionalLight();
    const SpotLight& spotLight = lightManager_->GetSpotLight();

    player_->SetDirectionalLight(dirLight);
    player_->SetSpotLight(spotLight);
    ground_->SetDirectionalLight(dirLight);
    ground_->SetSpotLight(spotLight);

    if (objeObject_) {
        objeObject_->SetDirectionalLight(dirLight);
        objeObject_->SetSpotLight(spotLight);
        objeObject_->Update();
    }

    if (skyboxEnabled_ && skybox_) {
        skybox_->Update();
    }
    player_->Update(engine);
    ground_->Update();
}

void GamePlayScene::Draw() {
    if (skyboxEnabled_ && skybox_) {
        skybox_->Draw(camera_);
    }

    spriteCommon_->CommonDraw();

    ground_->Draw();
    player_->Draw();

    if (objeObject_) {
        objeObject_->Draw();
    }

    player_->DrawUI();

    if (lightManager_) {
        lightManager_->DrawImGui();
    }

    // コリジョンデバッグUI
    auto* collisionManager = Collision::AABBCollisionManager::GetInstance();
    if (collisionManager) {
        collisionManager->DrawImGui();
    }
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
    objeObject_.reset();
    objeModel_.reset();
    skybox_.reset();
    lightManager_.reset();
}

void GamePlayScene::HandleInput() {
    UnoEngine* engine = UnoEngine::GetInstance();

    if (engine->IsKeyTriggered(DIK_F)) {
        lightManager_->ToggleDebugDisplay();
    }
}

void GamePlayScene::UpdateCamera() {
}

void GamePlayScene::DrawUI() {
}