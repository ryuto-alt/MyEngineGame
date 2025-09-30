#include "TitleScene.h"
#include "../../Engine/Resource/ResourcePreloader.h"
#include "SceneManager.h"
#include "imgui.h"

void TitleScene::Initialize() {
    camera_->SetTranslate({0.0f, 0.0f, -10.0f});

    ResourcePreloader::GetInstance()->PreloadAnimatedModelLightweight("human_walk", "Resources/Models/human", "walk.gltf", dxCommon_);
    ResourcePreloader::GetInstance()->PreloadAnimatedModelLightweight("human_sneak", "Resources/Models/human", "sneakWalk.gltf", dxCommon_);
}

void TitleScene::Update() {
    camera_->Update();

    if (input_->TriggerKey(DIK_SPACE)) {
        sceneManager_->ChangeScene("GamePlay");
    }

    if (input_->TriggerKey(DIK_ESCAPE)) {
        exit(0);
    }
}

void TitleScene::Draw() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 200, ImGui::GetIO().DisplaySize.y * 0.3f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
    ImGui::Begin("Title", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("MY ENGINE GAME");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Start Game (SPACE)", ImVec2(350, 40))) {
        sceneManager_->ChangeScene("GamePlay");
    }

    if (ImGui::Button("Exit (ESC)", ImVec2(350, 40))) {
        exit(0);
    }

    ImGui::End();
}