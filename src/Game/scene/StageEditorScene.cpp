#include "StageEditorScene.h"
#include "../../Engine/Stage/StageSerializer.h"
#include "../../Engine/Stage/StageBuilder.h"
#include <imgui.h>
#include <cstring>
#include <cmath>
#include <algorithm>

StageEditorScene::StageEditorScene() {
}

StageEditorScene::~StageEditorScene() {
}

void StageEditorScene::Initialize() {
    if (initialized_) return;

    // ステージビルダーの初期化
    stageBuilder_ = &StageBuilder::GetInstance();
    stageBuilder_->Initialize();
    stageBuilder_->SetGraphicsContext(dxCommon_, spriteCommon_);
    stageBuilder_->CreateNewStage(newStageWidth_, newStageHeight_);

    // カメラの初期設定
    Vector3 cameraPos = {0.0f, cameraDistance_ * sin(cameraPitch_ * 3.14159f / 180.0f), 
                        -cameraDistance_ * cos(cameraPitch_ * 3.14159f / 180.0f)};
    camera_->SetTranslate(cameraPos);
    camera_->SetRotate({cameraPitch_ * 3.14159f / 180.0f, cameraAngle_ * 3.14159f / 180.0f, 0.0f});

    // 利用可能なステージリストを取得
    availableStages_ = StageSerializer::GetAvailableStages();

    initialized_ = true;
}

void StageEditorScene::Update() {
    // 入力処理
    HandleMouseInput();
    HandleKeyboardInput();

    // マウスのグリッド位置を更新
    UpdateMouseGridPosition();

    // カメラ更新
    UpdateCamera();

    // ステージビルダー更新
    stageBuilder_->Update();

    // プレビューオブジェクトの更新
    if (previewObject_ && isValidPlacement_) {
        Vector3 worldPos = stageBuilder_->GetGrid()->GridToWorldPosition(mouseGridX_, mouseGridY_);
        previewObject_->SetPosition(worldPos);
        // previewObject_->SetAlpha(0.5f);  // 半透明表示（APIが存在しない）
        previewObject_->Update();
    }
}

void StageEditorScene::Draw() {
    // ステージの描画
    stageBuilder_->Draw();

    // プレビューの描画
    if (previewObject_ && isValidPlacement_) {
        previewObject_->Draw();
    }

    // ImGUIの描画
    DrawImGui();
}

void StageEditorScene::Finalize() {
    if (stageBuilder_) {
        stageBuilder_->Finalize();
    }
    initialized_ = false;
}

void StageEditorScene::DrawImGui() {
    // メインメニューバー
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Stage")) {
                stageBuilder_->CreateNewStage(newStageWidth_, newStageHeight_);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save")) {
                std::string filePath = StageSerializer::GetStageFilePath(saveFileName_);
                StageSerializer::SaveStage(filePath, stageBuilder_);
            }
            if (ImGui::MenuItem("Load")) {
                // ロードダイアログを表示
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                SceneManager::GetInstance()->ChangeScene("TitleScene");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Grid", nullptr, &showGrid_);
            ImGui::MenuItem("Show Palette", nullptr, &showPalette_);
            ImGui::MenuItem("Show Info", nullptr, &showInfo_);
            bool debugDraw = stageBuilder_->IsDebugDrawEnabled();
            if (ImGui::MenuItem("Debug Draw", nullptr, &debugDraw)) {
                stageBuilder_->SetDebugDrawEnabled(debugDraw);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // ツールバー
    DrawToolbar();

    // パーツパレット
    if (showPalette_) {
        DrawPartPalette();
    }

    // グリッドビュー
    DrawGridView();

    // ステージ情報
    if (showInfo_) {
        DrawStageInfo();
    }
}

void StageEditorScene::DrawPartPalette() {
    ImGui::SetNextWindowPos(ImVec2(10, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Parts Palette", &showPalette_)) {
        ImGui::Text("Select Part:");
        ImGui::Separator();

        // パーツカテゴリごとに表示
        if (ImGui::CollapsingHeader("Floor")) {
            if (ImGui::Selectable("Basic Floor", stageBuilder_->GetSelectedPartType() == StagePartType::FLOOR_BASIC)) {
                stageBuilder_->SetSelectedPartType(StagePartType::FLOOR_BASIC);
            }
            if (ImGui::Selectable("Damaged Floor", stageBuilder_->GetSelectedPartType() == StagePartType::FLOOR_DAMAGED)) {
                stageBuilder_->SetSelectedPartType(StagePartType::FLOOR_DAMAGED);
            }
            if (ImGui::Selectable("Special Floor", stageBuilder_->GetSelectedPartType() == StagePartType::FLOOR_SPECIAL)) {
                stageBuilder_->SetSelectedPartType(StagePartType::FLOOR_SPECIAL);
            }
        }

        if (ImGui::CollapsingHeader("Wall")) {
            if (ImGui::Selectable("Straight Wall", stageBuilder_->GetSelectedPartType() == StagePartType::WALL_STRAIGHT)) {
                stageBuilder_->SetSelectedPartType(StagePartType::WALL_STRAIGHT);
            }
            if (ImGui::Selectable("Corner Wall", stageBuilder_->GetSelectedPartType() == StagePartType::WALL_CORNER)) {
                stageBuilder_->SetSelectedPartType(StagePartType::WALL_CORNER);
            }
            if (ImGui::Selectable("T-Junction", stageBuilder_->GetSelectedPartType() == StagePartType::WALL_T_JUNCTION)) {
                stageBuilder_->SetSelectedPartType(StagePartType::WALL_T_JUNCTION);
            }
            if (ImGui::Selectable("Cross Junction", stageBuilder_->GetSelectedPartType() == StagePartType::WALL_CROSS)) {
                stageBuilder_->SetSelectedPartType(StagePartType::WALL_CROSS);
            }
            if (ImGui::Selectable("Door", stageBuilder_->GetSelectedPartType() == StagePartType::WALL_DOOR)) {
                stageBuilder_->SetSelectedPartType(StagePartType::WALL_DOOR);
            }
        }

        if (ImGui::CollapsingHeader("Ceiling")) {
            if (ImGui::Selectable("Basic Ceiling", stageBuilder_->GetSelectedPartType() == StagePartType::CEILING_BASIC)) {
                stageBuilder_->SetSelectedPartType(StagePartType::CEILING_BASIC);
            }
            if (ImGui::Selectable("Light Ceiling", stageBuilder_->GetSelectedPartType() == StagePartType::CEILING_LIGHT)) {
                stageBuilder_->SetSelectedPartType(StagePartType::CEILING_LIGHT);
            }
        }

        if (ImGui::CollapsingHeader("Props")) {
            if (ImGui::Selectable("Stairs Up", stageBuilder_->GetSelectedPartType() == StagePartType::STAIRS_UP)) {
                stageBuilder_->SetSelectedPartType(StagePartType::STAIRS_UP);
            }
            if (ImGui::Selectable("Stairs Down", stageBuilder_->GetSelectedPartType() == StagePartType::STAIRS_DOWN)) {
                stageBuilder_->SetSelectedPartType(StagePartType::STAIRS_DOWN);
            }
            if (ImGui::Selectable("Pillar", stageBuilder_->GetSelectedPartType() == StagePartType::PILLAR)) {
                stageBuilder_->SetSelectedPartType(StagePartType::PILLAR);
            }
            if (ImGui::Selectable("Decoration", stageBuilder_->GetSelectedPartType() == StagePartType::DECORATION)) {
                stageBuilder_->SetSelectedPartType(StagePartType::DECORATION);
            }
        }

        ImGui::Separator();
        ImGui::Text("Rotation: %d°", stageBuilder_->GetSelectedRotation());
        if (ImGui::Button("Rotate Left (Q)")) {
            int rot = stageBuilder_->GetSelectedRotation() - 90;
            if (rot < 0) rot += 360;
            stageBuilder_->SetSelectedRotation(rot);
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate Right (E)")) {
            int rot = (stageBuilder_->GetSelectedRotation() + 90) % 360;
            stageBuilder_->SetSelectedRotation(rot);
        }
    }
    ImGui::End();
}

void StageEditorScene::DrawGridView() {
    ImGui::SetNextWindowPos(ImVec2(220, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Grid View (Top Down)")) {
        // グリッドのトップダウンビューを描画
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
        if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;

        // グリッドの描画
        auto grid = stageBuilder_->GetGrid();
        if (grid) {
            float cellSize = (std::min)(canvasSize.x / grid->GetWidth(), canvasSize.y / grid->GetHeight());
            
            // グリッドライン
            for (int x = 0; x <= grid->GetWidth(); x++) {
                drawList->AddLine(
                    ImVec2(canvasPos.x + x * cellSize, canvasPos.y),
                    ImVec2(canvasPos.x + x * cellSize, canvasPos.y + grid->GetHeight() * cellSize),
                    IM_COL32(100, 100, 100, 255)
                );
            }
            for (int y = 0; y <= grid->GetHeight(); y++) {
                drawList->AddLine(
                    ImVec2(canvasPos.x, canvasPos.y + y * cellSize),
                    ImVec2(canvasPos.x + grid->GetWidth() * cellSize, canvasPos.y + y * cellSize),
                    IM_COL32(100, 100, 100, 255)
                );
            }

            // 配置されたパーツ
            for (int y = 0; y < grid->GetHeight(); y++) {
                for (int x = 0; x < grid->GetWidth(); x++) {
                    auto part = grid->GetPart(x, y);
                    if (part) {
                        ImU32 color = IM_COL32(100, 200, 100, 255);  // 緑色
                        
                        // パーツタイプによって色を変える
                        switch (part->GetType()) {
                        case StagePartType::WALL_STRAIGHT:
                        case StagePartType::WALL_CORNER:
                        case StagePartType::WALL_T_JUNCTION:
                        case StagePartType::WALL_CROSS:
                            color = IM_COL32(200, 100, 100, 255);  // 赤色
                            break;
                        case StagePartType::WALL_DOOR:
                            color = IM_COL32(100, 100, 200, 255);  // 青色
                            break;
                        }
                        
                        drawList->AddRectFilled(
                            ImVec2(canvasPos.x + x * cellSize + 1, canvasPos.y + y * cellSize + 1),
                            ImVec2(canvasPos.x + (x + 1) * cellSize - 1, canvasPos.y + (y + 1) * cellSize - 1),
                            color
                        );
                    }
                }
            }

            // マウスホバー位置
            if (isValidPlacement_) {
                drawList->AddRect(
                    ImVec2(canvasPos.x + mouseGridX_ * cellSize, canvasPos.y + mouseGridY_ * cellSize),
                    ImVec2(canvasPos.x + (mouseGridX_ + 1) * cellSize, canvasPos.y + (mouseGridY_ + 1) * cellSize),
                    IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f
                );
            }
        }
    }
    ImGui::End();
}

void StageEditorScene::DrawToolbar() {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(800, 60), ImGuiCond_Always);
    
    if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
        // 新規作成
        ImGui::Text("New Stage Size:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::InputInt("W", &newStageWidth_);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::InputInt("H", &newStageHeight_);
        ImGui::SameLine();
        if (ImGui::Button("Create")) {
            stageBuilder_->CreateNewStage(newStageWidth_, newStageHeight_);
        }

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // 保存・読み込み
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##filename", saveFileName_, sizeof(saveFileName_));
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            std::string filePath = StageSerializer::GetStageFilePath(saveFileName_);
            if (StageSerializer::SaveStage(filePath, stageBuilder_)) {
                availableStages_ = StageSerializer::GetAvailableStages();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            if (selectedStageIndex_ >= 0 && selectedStageIndex_ < availableStages_.size()) {
                std::string filePath = StageSerializer::GetStageFilePath(availableStages_[selectedStageIndex_]);
                StageSerializer::LoadStage(filePath, stageBuilder_);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
                strcpy(saveFileName_, availableStages_[selectedStageIndex_].c_str());
#ifdef _MSC_VER
#pragma warning(pop)
#endif
            }
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        if (ImGui::BeginCombo("##stages", selectedStageIndex_ >= 0 ? availableStages_[selectedStageIndex_].c_str() : "Select Stage")) {
            for (int i = 0; i < availableStages_.size(); i++) {
                bool isSelected = (selectedStageIndex_ == i);
                if (ImGui::Selectable(availableStages_[i].c_str(), isSelected)) {
                    selectedStageIndex_ = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // カメラモード
        if (ImGui::Button(isTopViewMode_ ? "3D View" : "Top View")) {
            isTopViewMode_ = !isTopViewMode_;
        }
    }
    ImGui::End();
}

void StageEditorScene::DrawStageInfo() {
    ImGui::SetNextWindowPos(ImVec2(10, 520), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Stage Info", &showInfo_)) {
        auto grid = stageBuilder_->GetGrid();
        if (grid) {
            ImGui::Text("Stage: %s", stageBuilder_->GetStageName().c_str());
            ImGui::Text("Size: %d x %d", grid->GetWidth(), grid->GetHeight());
            ImGui::Text("Mouse Grid: %d, %d", mouseGridX_, mouseGridY_);
            ImGui::Text("Camera Mode: %s", isTopViewMode_ ? "Top View" : "3D View");
            
            if (stageBuilder_->ValidateStageConnectivity()) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Stage is connected");
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Stage has disconnected parts!");
            }
        }
    }
    ImGui::End();
}

void StageEditorScene::HandleMouseInput() {
    // 左クリックで配置（マウスボタンは別の方法で取得）
    // if (input_->TriggerMouse(GLFW_MOUSE_BUTTON_LEFT) && isValidPlacement_) {
    //     stageBuilder_->PlacePart(mouseGridX_, mouseGridY_, 
    //                             stageBuilder_->GetSelectedPartType(), 
    //                             stageBuilder_->GetSelectedRotation());
    // }

    // 右クリックで削除（マウスボタンは別の方法で取得）
    // if (input_->TriggerMouse(GLFW_MOUSE_BUTTON_RIGHT)) {
    //     stageBuilder_->RemovePart(mouseGridX_, mouseGridY_);
    // }

    // スクロールでカメラズーム（3Dビューモードのみ）
    if (!isTopViewMode_) {
        // float scroll = input_->GetWheel();
        // if (scroll != 0) {
        //     cameraDistance_ -= scroll * 5.0f;
        //     cameraDistance_ = (std::max)(10.0f, (std::min)(100.0f, cameraDistance_));
        // }
    }
}

void StageEditorScene::HandleKeyboardInput() {
    // Q/Eで回転
    if (input_->TriggerKey(DIK_Q)) {
        int rot = stageBuilder_->GetSelectedRotation() - 90;
        if (rot < 0) rot += 360;
        stageBuilder_->SetSelectedRotation(rot);
    }
    if (input_->TriggerKey(DIK_E)) {
        int rot = (stageBuilder_->GetSelectedRotation() + 90) % 360;
        stageBuilder_->SetSelectedRotation(rot);
    }

    // Rでパーツを回転（配置済み）
    if (input_->TriggerKey(DIK_R)) {
        stageBuilder_->RotatePart(mouseGridX_, mouseGridY_, 90);
    }

    // Deleteで削除
    if (input_->TriggerKey(DIK_DELETE)) {
        stageBuilder_->RemovePart(mouseGridX_, mouseGridY_);
    }

    // カメラ操作（3Dビューモードのみ）
    if (!isTopViewMode_) {
        if (input_->PushKey(DIK_A)) cameraAngle_ -= 2.0f;
        if (input_->PushKey(DIK_D)) cameraAngle_ += 2.0f;
        if (input_->PushKey(DIK_W)) cameraPitch_ = (std::min)(89.0f, cameraPitch_ + 2.0f);
        if (input_->PushKey(DIK_S)) cameraPitch_ = (std::max)(10.0f, cameraPitch_ - 2.0f);
    }
}

void StageEditorScene::UpdateMouseGridPosition() {
    auto grid = stageBuilder_->GetGrid();
    if (!grid) return;

    // レイキャストまたはスクリーン座標からグリッド座標を計算
    // TODO: 実際のレイキャスト実装
    
    // 仮実装：画面中央付近のグリッド位置を使用
    mouseGridX_ = grid->GetWidth() / 2;
    mouseGridY_ = grid->GetHeight() / 2;
    
    isValidPlacement_ = grid->IsValidPosition(mouseGridX_, mouseGridY_) &&
                       grid->GetPart(mouseGridX_, mouseGridY_) == nullptr;
}

void StageEditorScene::UpdateCamera() {
    if (isTopViewMode_) {
        // トップビューモード
        Vector3 cameraPos = {0.0f, 50.0f, -1.0f};  // ほぼ真上から
        camera_->SetTranslate(cameraPos);
        camera_->SetRotate({89.0f * 3.14159f / 180.0f, 0.0f, 0.0f});
    } else {
        // 3Dビューモード
        float radAngle = cameraAngle_ * 3.14159f / 180.0f;
        float radPitch = cameraPitch_ * 3.14159f / 180.0f;
        
        Vector3 cameraPos = {
            cameraDistance_ * sin(radAngle) * cos(radPitch),
            cameraDistance_ * sin(radPitch),
            -cameraDistance_ * cos(radAngle) * cos(radPitch)
        };
        
        camera_->SetTranslate(cameraPos);
        camera_->SetRotate({radPitch, radAngle, 0.0f});
    }
}