#include "MazeManager.h"
#include "imgui.h"
#include "Mymath.h"
#include <fstream>
#include <sstream>
#include <cmath>

MazeManager::MazeManager()
    : currentWidth_(20)
    , currentHeight_(20)
    , currentCellSize_(2.0f)
    , playerPosition_(0, 0, 0)
    , playerGridPos_(0, 0)
    , darkDeceptionMode_(true)
    , horrorIntensity_(0.5f)
    , jumpScareTimer_(0.0f)
    , showDebugInfo_(false)
    , dxCommon_(nullptr)
    , spriteCommon_(nullptr) {
    
    mazeGrid_ = std::make_unique<MazeGrid>(currentWidth_, currentHeight_, currentCellSize_);
    mazeGenerator_ = std::make_unique<MazeGenerator>();
    mazeRenderer_ = std::make_unique<MazeRenderer>();
    
    // デフォルトのDark Deceptionパラメータ設定
    currentParams_.algorithm = MazeAlgorithm::DarkDeceptionStyle;
    currentParams_.wallDensity = 0.3f;
    currentParams_.branchingFactor = 0.3f;
    currentParams_.straightBias = 0.7f;
    currentParams_.minCorridorLength = 3;
    currentParams_.maxCorridorLength = 8;
    currentParams_.createLoops = true;
    currentParams_.loopProbability = 0.15f;
    currentParams_.addRooms = true;
    currentParams_.roomCount = 4;
    currentParams_.minRoomSize = 3;
    currentParams_.maxRoomSize = 6;
}

MazeManager::~MazeManager() {
}

void MazeManager::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    
    // レンダラーを初期化
    mazeRenderer_->Initialize(dxCommon, spriteCommon);
    
    // Dark Deceptionエフェクトを設定
    if (darkDeceptionMode_) {
        mazeRenderer_->SetFogEnabled(true);
        mazeRenderer_->SetFogColor(Vector4(0.05f, 0.05f, 0.08f, 1.0f));
        mazeRenderer_->SetFogDensity(0.08f);
        mazeRenderer_->SetFogStart(3.0f);
        mazeRenderer_->SetFogEnd(25.0f);
        mazeRenderer_->SetFlickerLightEnabled(true);
        mazeRenderer_->SetAmbientLight(Vector3(0.05f, 0.05f, 0.08f));
    }
}

void MazeManager::GenerateMaze(int width, int height, float cellSize) {
    currentWidth_ = width;
    currentHeight_ = height;
    currentCellSize_ = cellSize;
    
    // 新しいグリッドを作成
    mazeGrid_ = std::make_unique<MazeGrid>(width, height, cellSize);
    
    // 迷路を生成
    mazeGenerator_->Generate(mazeGrid_.get(), currentParams_);
    
    // レンダラーで迷路を構築
    mazeRenderer_->BuildMaze(mazeGrid_.get());
    
    // プレイヤーをスタート地点に配置
    Vector2 startPos = mazeGrid_->GetStartPosition();
    Vector3 worldPos = mazeGrid_->GridToWorld(startPos);
    SetPlayerPosition(worldPos);
}

void MazeManager::GenerateMaze(const MazeGenerator::GenerationParams& params) {
    currentParams_ = params;
    GenerateMaze(currentWidth_, currentHeight_, currentCellSize_);
}

bool MazeManager::LoadMaze(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    
    // JSONから迷路を読み込み
    if (mazeGrid_->ImportFromJSON(json)) {
        // レンダラーで迷路を再構築
        mazeRenderer_->BuildMaze(mazeGrid_.get());
        return true;
    }
    
    return false;
}

bool MazeManager::SaveMaze(const std::string& filepath) {
    std::string json = mazeGrid_->ExportToJSON();
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    file << json;
    return true;
}

void MazeManager::Update(Camera* camera, float deltaTime) {
    // プレイヤーのグリッド位置を更新
    UpdatePlayerGridPosition();
    
    // レンダラーを更新
    mazeRenderer_->Update(camera);
    
    // Dark Deceptionエフェクトを更新
    if (darkDeceptionMode_) {
        mazeRenderer_->UpdateDarkDeceptionEffects(deltaTime);
        
        // ジャンプスケアタイマーを更新
        if (jumpScareTimer_ > 0.0f) {
            jumpScareTimer_ -= deltaTime;
            
            // ジャンプスケア中はホラー強度を上げる
            mazeRenderer_->SetFogDensity(0.15f + jumpScareTimer_ * 0.2f);
        }
    }
    
    // 敵のAIを更新
    UpdateEnemyAI(deltaTime);
    
    // 衝突判定
    CheckCollisions();
}

void MazeManager::Draw() {
    mazeRenderer_->Draw();
}

void MazeManager::SetPlayerPosition(const Vector3& position) {
    playerPosition_ = position;
    UpdatePlayerGridPosition();
}

Vector3 MazeManager::GetPlayerGridPosition() const {
    return Vector3(playerGridPos_.x, 0, playerGridPos_.y);
}

bool MazeManager::IsPlayerAtGoal() const {
    Vector2 goalPos = mazeGrid_->GetGoalPosition();
    return (static_cast<int>(playerGridPos_.x) == static_cast<int>(goalPos.x) &&
            static_cast<int>(playerGridPos_.y) == static_cast<int>(goalPos.y));
}

bool MazeManager::CanMoveTo(const Vector3& position) const {
    Vector2 gridPos = mazeGrid_->WorldToGrid(position);
    int x = static_cast<int>(gridPos.x);
    int y = static_cast<int>(gridPos.y);
    
    if (!mazeGrid_->IsValidPosition(x, y)) {
        return false;
    }
    
    MazeCell* cell = mazeGrid_->GetCell(x, y);
    if (!cell) {
        return false;
    }
    
    // 壁には移動できない
    return cell->GetType() != MazeCellType::Wall && cell->GetType() != MazeCellType::Empty;
}

void MazeManager::PlaceItems(int count) {
    itemPositions_.clear();
    itemCollected_.clear();
    
    mazeGenerator_->PlaceItems(mazeGrid_.get(), count);
    
    // アイテム位置を収集
    mazeGrid_->ForEachCell([this](int x, int y, MazeCell* cell) {
        if (cell && cell->GetType() == MazeCellType::Item) {
            Vector3 worldPos = mazeGrid_->GridToWorld(x, y);
            itemPositions_.push_back(worldPos);
            itemCollected_.push_back(false);
        }
    });
}

void MazeManager::PlaceEnemies(int count) {
    enemyPositions_.clear();
    
    mazeGenerator_->PlaceEnemies(mazeGrid_.get(), count);
    
    // 敵の位置を収集
    mazeGrid_->ForEachCell([this](int x, int y, MazeCell* cell) {
        if (cell && cell->GetType() == MazeCellType::Enemy) {
            Vector3 worldPos = mazeGrid_->GridToWorld(x, y);
            enemyPositions_.push_back(worldPos);
        }
    });
}

bool MazeManager::CheckItemCollection(const Vector3& position) {
    for (size_t i = 0; i < itemPositions_.size(); ++i) {
        if (!itemCollected_[i]) {
            float distance = Vector3Length(position - itemPositions_[i]);
            if (distance < 1.0f) { // 収集範囲
                itemCollected_[i] = true;
                return true;
            }
        }
    }
    return false;
}

std::vector<Vector3> MazeManager::GetEnemyPositions() const {
    return enemyPositions_;
}

void MazeManager::EnableDarkDeceptionMode(bool enable) {
    darkDeceptionMode_ = enable;
    
    if (enable) {
        mazeRenderer_->SetFogEnabled(true);
        mazeRenderer_->SetFlickerLightEnabled(true);
        mazeRenderer_->SetAmbientLight(Vector3(0.05f, 0.05f, 0.08f));
    } else {
        mazeRenderer_->SetFogEnabled(false);
        mazeRenderer_->SetFlickerLightEnabled(false);
        mazeRenderer_->SetAmbientLight(Vector3(0.3f, 0.3f, 0.3f));
    }
}

void MazeManager::SetHorrorIntensity(float intensity) {
    horrorIntensity_ = intensity;
    
    // ホラー強度に応じてエフェクトを調整
    float fogDensity = 0.05f + intensity * 0.1f;
    float fogEnd = 30.0f - intensity * 15.0f;
    Vector3 ambientLight(0.1f - intensity * 0.08f, 
                         0.1f - intensity * 0.08f, 
                         0.12f - intensity * 0.08f);
    
    mazeRenderer_->SetFogDensity(fogDensity);
    mazeRenderer_->SetFogEnd(fogEnd);
    mazeRenderer_->SetAmbientLight(ambientLight);
}

void MazeManager::TriggerJumpScare(const Vector3& position) {
    jumpScarePosition_ = position;
    jumpScareTimer_ = 1.0f; // 1秒間のジャンプスケア
    
    // 一時的にホラー強度を最大に
    SetHorrorIntensity(1.0f);
}

void MazeManager::RegenerateMaze() {
    GenerateMaze(currentWidth_, currentHeight_, currentCellSize_);
}

void MazeManager::DrawImGuiDebugWindow() {
    if (!showDebugInfo_) return;
    
    ImGui::Begin("Maze Manager Debug");
    
    // 迷路生成パラメータ
    ImGui::Text("Maze Generation");
    ImGui::SliderInt("Width", &currentWidth_, 10, 50);
    ImGui::SliderInt("Height", &currentHeight_, 10, 50);
    ImGui::SliderFloat("Cell Size", &currentCellSize_, 1.0f, 5.0f);
    
    // アルゴリズム選択
    const char* algorithms[] = {
        "Recursive Backtracking",
        "Prim",
        "Kruskal",
        "Recursive Division",
        "Binary Tree",
        "Dark Deception Style"
    };
    int currentAlgo = static_cast<int>(currentParams_.algorithm);
    if (ImGui::Combo("Algorithm", &currentAlgo, algorithms, IM_ARRAYSIZE(algorithms))) {
        currentParams_.algorithm = static_cast<MazeAlgorithm>(currentAlgo);
    }
    
    // 生成パラメータ
    ImGui::SliderFloat("Wall Density", &currentParams_.wallDensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Branching Factor", &currentParams_.branchingFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Straight Bias", &currentParams_.straightBias, 0.0f, 1.0f);
    ImGui::Checkbox("Create Loops", &currentParams_.createLoops);
    if (currentParams_.createLoops) {
        ImGui::SliderFloat("Loop Probability", &currentParams_.loopProbability, 0.0f, 0.5f);
    }
    ImGui::Checkbox("Add Rooms", &currentParams_.addRooms);
    if (currentParams_.addRooms) {
        ImGui::SliderInt("Room Count", &currentParams_.roomCount, 0, 10);
    }
    
    // 再生成ボタン
    if (ImGui::Button("Regenerate Maze")) {
        RegenerateMaze();
    }
    
    ImGui::Separator();
    
    // Dark Deceptionエフェクト
    ImGui::Text("Dark Deception Effects");
    if (ImGui::Checkbox("Enable Dark Mode", &darkDeceptionMode_)) {
        EnableDarkDeceptionMode(darkDeceptionMode_);
    }
    
    if (darkDeceptionMode_) {
        if (ImGui::SliderFloat("Horror Intensity", &horrorIntensity_, 0.0f, 1.0f)) {
            SetHorrorIntensity(horrorIntensity_);
        }
        
        if (ImGui::Button("Trigger Jump Scare")) {
            TriggerJumpScare(playerPosition_);
        }
    }
    
    ImGui::Separator();
    
    // レンダリング設定
    ImGui::Text("Rendering");
    bool fogEnabled = mazeRenderer_->GetStatistics().totalPieces > 0;
    if (ImGui::Checkbox("Enable Fog", &fogEnabled)) {
        mazeRenderer_->SetFogEnabled(fogEnabled);
    }
    
    float viewDistance = 50.0f;
    if (ImGui::SliderFloat("View Distance", &viewDistance, 10.0f, 100.0f)) {
        mazeRenderer_->SetViewDistance(viewDistance);
    }
    
    ImGui::Separator();
    
    // 統計情報
    ImGui::Text("Statistics");
    auto gridStats = mazeGrid_->GetStatistics();
    ImGui::Text("Total Cells: %d", gridStats.totalCells);
    ImGui::Text("Walls: %d", gridStats.wallCount);
    ImGui::Text("Paths: %d", gridStats.pathCount);
    
    auto renderStats = mazeRenderer_->GetStatistics();
    ImGui::Text("Total Pieces: %d", renderStats.totalPieces);
    ImGui::Text("Visible Pieces: %d", renderStats.visiblePieces);
    ImGui::Text("Culled Pieces: %d", renderStats.culledPieces);
    
    ImGui::Text("Player Grid Pos: (%.0f, %.0f)", playerGridPos_.x, playerGridPos_.y);
    ImGui::Text("At Goal: %s", IsPlayerAtGoal() ? "Yes" : "No");
    
    ImGui::End();
}

void MazeManager::UpdatePlayerGridPosition() {
    playerGridPos_ = mazeGrid_->WorldToGrid(playerPosition_);
}

void MazeManager::UpdateEnemyAI(float deltaTime) {
    // 簡単な敵AI：プレイヤーに向かって移動
    for (auto& enemyPos : enemyPositions_) {
        Vector3 toPlayer = playerPosition_ - enemyPos;
        float distance = Vector3Length(toPlayer);
        
        if (distance > 0.1f && distance < 20.0f) { // 追跡範囲内
            Vector3 direction = Vector3Normalize(toPlayer);
            Vector3 newPos = enemyPos + direction * 2.0f * deltaTime; // 敵の速度
            
            // 移動可能かチェック
            if (CanMoveTo(newPos)) {
                enemyPos = newPos;
            }
        }
    }
}

void MazeManager::CheckCollisions() {
    // アイテム収集チェック
    CheckItemCollection(playerPosition_);
    
    // 敵との衝突チェック
    for (const auto& enemyPos : enemyPositions_) {
        float distance = Vector3Length(playerPosition_ - enemyPos);
        if (distance < 1.0f) { // 衝突範囲
            // ジャンプスケアを発動
            if (darkDeceptionMode_) {
                TriggerJumpScare(enemyPos);
            }
        }
    }
}