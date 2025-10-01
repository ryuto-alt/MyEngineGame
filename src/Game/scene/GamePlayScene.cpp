#include "GamePlayScene.h"
#include "imgui.h"
#include "UnoEngine.h"
#include "SceneManager.h"

void GamePlayScene::Initialize() {
    UnoEngine* engine = UnoEngine::GetInstance();

    camera_->SetFov(1.37f);
    engine->SetCameraStickSensitivity(2.5f);
    engine->SetCameraOrbitDistance(3.0f);
    engine->SetCameraOrbitHeight(2.5f);
    engine->SetCameraOrbitTarget(Vector3{0.0f, 0.0f, 0.0f});

    lightManager_ = std::make_unique<LightManager>();
    lightManager_->Initialize();

    player_ = std::make_unique<Player>();
    player_->Initialize(camera_);

    ground_ = std::make_unique<Ground>();
    ground_->Initialize(camera_, dxCommon_);

    skyboxEnabled_ = false;

    if (player_) {
        player_->SetEnableEnvironmentMap(false);
    }
}

void GamePlayScene::Update() {
    UnoEngine* engine = UnoEngine::GetInstance();

    HandleInput();
    UpdateCamera();

    lightManager_->Update();
    const DirectionalLight& dirLight = lightManager_->GetDirectionalLight();
    const SpotLight& spotLight = lightManager_->GetSpotLight();

    player_->SetDirectionalLight(dirLight);
    player_->SetSpotLight(spotLight);
    ground_->SetDirectionalLight(dirLight);
    ground_->SetSpotLight(spotLight);

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

    DrawUI();

    if (lightManager_) {
        lightManager_->DrawImGui();
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
    skybox_.reset();
    lightManager_.reset();
}

void GamePlayScene::HandleInput() {
    UnoEngine* engine = UnoEngine::GetInstance();
    float deltaTime = engine->GetDeltaTime();

    if (engine->IsKeyTriggered(DIK_ESCAPE)) {
        engine->RequestEnd();
        return;
    }

#ifdef _DEBUG
    if (engine->IsKeyTriggered(DIK_TAB)) {
        camera_->ToggleMouseLook();
    }
    if (engine->IsKeyTriggered(DIK_F1)) {
        camera_->ToggleFreeCameraMode();
    }
#endif

    if (engine->IsKeyTriggered(DIK_F)) {
        lightManager_->ToggleDebugDisplay();
    }

    if (engine->IsKeyTriggered(DIK_P)) {
        if (player_->IsAnimationPaused()) {
            player_->PlayAnimation();
        } else {
            player_->PauseAnimation();
        }
    }
    if (engine->IsKeyTriggered(DIK_R)) {
        player_->ResetAnimation();
    }
    if (engine->IsKeyTriggered(DIK_1) || engine->IsKeyTriggered(DIK_RSHIFT)) {
        player_->ToggleSneakWalk();
    }

#ifdef _DEBUG
    if (camera_->IsFreeCameraMode()) {
        float cameraSpeed = 5.0f * deltaTime;
        if (engine->IsKeyPressed(DIK_W)) camera_->MoveForward(cameraSpeed);
        if (engine->IsKeyPressed(DIK_S)) camera_->MoveForward(-cameraSpeed);
        if (engine->IsKeyPressed(DIK_A)) camera_->MoveRight(-cameraSpeed);
        if (engine->IsKeyPressed(DIK_D)) camera_->MoveRight(cameraSpeed);
        if (engine->IsKeyPressed(DIK_SPACE)) camera_->MoveUp(cameraSpeed);
        if (engine->IsKeyPressed(DIK_LSHIFT)) camera_->MoveUp(-cameraSpeed);
    } else
#endif
    {
        float forward = 0.0f;
        float right = 0.0f;

        if (engine->IsKeyPressed(DIK_W)) forward += 1.0f;
        if (engine->IsKeyPressed(DIK_S)) forward -= 1.0f;
        if (engine->IsKeyPressed(DIK_A)) right -= 1.0f;
        if (engine->IsKeyPressed(DIK_D)) right += 1.0f;

        float stickX = engine->GetXboxLeftStickX();
        float stickY = engine->GetXboxLeftStickY();

        const float deadZone = 0.1f;
        if (abs(stickX) < deadZone) stickX = 0.0f;
        if (abs(stickY) < deadZone) stickY = 0.0f;

        forward += stickY;
        right += stickX;

        float totalMagnitude = std::sqrt(forward * forward + right * right);
        if (totalMagnitude > 1.0f) {
            forward /= totalMagnitude;
            right /= totalMagnitude;
        }

        bool isPlayerMoving = (forward != 0.0f || right != 0.0f);

        if (isPlayerMoving) {
            player_->MoveWithCameraDirection(forward, right, deltaTime);
        } else if (player_->IsMoving()) {
            player_->StopMoving();
        }
    }
}

void GamePlayScene::UpdateCamera() {
    UnoEngine* engine = UnoEngine::GetInstance();
    engine->UpdateCameraMouse();
    engine->UpdateCameraRightStick();

#ifdef _DEBUG
    if (!camera_->IsFreeCameraMode())
#endif
    {
        player_->UpdateCameraFollow();
    }

    camera_->Update();
}

void GamePlayScene::DrawUI() {
    ImGui::Begin("GamePlay Scene");

    ImGui::Text("Controls:");
    ImGui::Separator();
    ImGui::Text("WASD - Move Player");
    ImGui::Text("P - Pause/Resume Animation");
    ImGui::Text("R - Reset Animation");
    ImGui::Text("1/RShift - Toggle SneakWalk");
    ImGui::Text("F - Toggle Light Settings");
    ImGui::Text("ESC - Back to Title");
#ifdef _DEBUG
    ImGui::Text("F1 - Toggle Free Camera");
    ImGui::Text("TAB - Toggle Mouse Look");
#endif

    ImGui::Separator();

    if (ImGui::Button("Go to Title Scene")) {
        sceneManager_->ChangeScene("Title");
    }

    ImGui::Separator();
    Vector3 cameraPos = camera_->GetTranslate();
    ImGui::Text("Camera: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);

#ifdef _DEBUG
    ImGui::Text("Camera Mode: %s", camera_->IsFreeCameraMode() ? "Free" : "Follow Player");
#endif

    ImGui::Separator();
    ImGui::Text("Animation: %s", player_->GetCurrentAnimationName().c_str());
    ImGui::Text("State: %s", player_->IsMoving() ? "Moving" : "Idle");

    if (player_->IsBlending()) {
        ImGui::Text("Blending: %.1f%%", player_->GetBlendProgress() * 100.0f);
    }

    float smoothingSpeed = player_->GetRotationSmoothingSpeed();
    if (ImGui::SliderFloat("Smoothing Speed", &smoothingSpeed, 0.1f, 20.0f)) {
        player_->SetRotationSmoothingSpeed(smoothingSpeed);
    }

    ImGui::Separator();
    ImGui::Text("Rendering Options:");

    ImGui::Text("Skybox: %s", skyboxEnabled_ ? "Disabled (prevents crash)" : "Disabled");
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Note: Skybox disabled to fix rendering crash");

    ImGui::Separator();
    ImGui::Text("Environment Map:");
    ImGui::Text("Status: Disabled for stability");

    ImGui::End();
}