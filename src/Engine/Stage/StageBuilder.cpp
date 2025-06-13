#include "StageBuilder.h"
#include "../Graphics/Model.h"
#include <algorithm>
#include <queue>

StageBuilder& StageBuilder::GetInstance() {
    static StageBuilder instance;
    return instance;
}

void StageBuilder::Initialize() {
    grid_ = std::make_unique<StageGrid>();
    // PreloadPartModelsはSetGraphicsContextが呼ばれた後に実行される
}

void StageBuilder::SetGraphicsContext(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    
    // モデルをプリロード
    PreloadPartModels();
    
    // すでに作成されているパーツにもグラフィックスコンテキストを設定
    for (auto& part : allParts_) {
        if (part) {
            part->InitializeGraphics(dxCommon_, spriteCommon_);
        }
    }
}

void StageBuilder::Finalize() {
    ClearStage();
    partModels_.clear();
    grid_.reset();
}

void StageBuilder::CreateNewStage(int width, int height) {
    ClearStage();
    grid_->Initialize(width, height);
    stageName_ = "NewStage";
}

bool StageBuilder::PlacePart(int x, int y, StagePartType type, int rotation) {
    // 新しいパーツを作成
    auto part = CreatePart(type);
    if (!part) {
        return false;
    }

    // 回転を設定
    part->SetRotation(rotation);

    // グリッドに配置
    if (!grid_->PlacePart(x, y, part)) {
        return false;
    }

    // ワールド座標を設定
    Vector3 worldPos = grid_->GridToWorldPosition(x, y);
    part->SetWorldPosition(worldPos);

    // 管理リストに追加
    allParts_.push_back(part);

    return true;
}

bool StageBuilder::RemovePart(int x, int y) {
    auto part = grid_->GetPart(x, y);
    if (!part) {
        return false;
    }

    // グリッドから削除
    if (!grid_->RemovePart(x, y)) {
        return false;
    }

    // 管理リストから削除
    allParts_.erase(
        std::remove(allParts_.begin(), allParts_.end(), part),
        allParts_.end()
    );

    return true;
}

bool StageBuilder::RotatePart(int x, int y, int rotationDelta) {
    auto part = grid_->GetPart(x, y);
    if (!part) {
        return false;
    }

    int newRotation = (part->GetRotation() + rotationDelta) % 360;
    if (newRotation < 0) newRotation += 360;

    part->SetRotation(newRotation);
    return true;
}

void StageBuilder::ClearStage() {
    if (grid_) {
        grid_->Clear();
    }
    allParts_.clear();
}

bool StageBuilder::ValidateStageConnectivity() const {
    if (!grid_) return false;

    // BFSを使用して接続性をチェック
    int width = grid_->GetWidth();
    int height = grid_->GetHeight();
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));

    // 最初のパーツを見つける
    int startX = -1, startY = -1;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid_->GetPart(x, y)) {
                startX = x;
                startY = y;
                break;
            }
        }
        if (startX != -1) break;
    }

    if (startX == -1) return true;  // パーツがない場合は有効とする

    // BFS
    std::queue<std::pair<int, int>> queue;
    queue.push({startX, startY});
    visited[startY][startX] = true;
    int connectedCount = 1;

    const int dx[] = {0, 1, 0, -1};  // 北、東、南、西
    const int dy[] = {-1, 0, 1, 0};

    while (!queue.empty()) {
        auto [x, y] = queue.front();
        queue.pop();

        auto currentPart = grid_->GetPart(x, y);
        if (!currentPart) continue;

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
            if (visited[ny][nx]) continue;

            auto neighborPart = grid_->GetPart(nx, ny);
            if (!neighborPart) continue;

            // 接続可能性をチェック
            Direction dir = static_cast<Direction>(i);
            if (currentPart->IsCompatibleWith(neighborPart.get(), dir)) {
                visited[ny][nx] = true;
                queue.push({nx, ny});
                connectedCount++;
            }
        }
    }

    // すべてのパーツが接続されているかチェック
    int totalParts = 0;
    for (const auto& row : grid_->GetGrid()) {
        for (const auto& part : row) {
            if (part) totalParts++;
        }
    }

    return connectedCount == totalParts;
}

void StageBuilder::Update() {
    for (auto& part : allParts_) {
        if (part) {
            part->Update();
        }
    }
}

void StageBuilder::Draw() {
    for (auto& part : allParts_) {
        if (part) {
            part->Draw();
        }
    }

    if (debugDrawEnabled_) {
        DrawDebugGrid();
        DrawConnectionIndicators();
    }
}

void StageBuilder::PreloadPartModels() {
    // DirectXCommonが設定されていない場合はスキップ
    if (!dxCommon_) {
        return;
    }
    
    // 各パーツタイプのモデルをプリロード
    for (int i = 0; i < static_cast<int>(StagePartType::MAX_TYPE); ++i) {
        StagePartType type = static_cast<StagePartType>(i);
        std::string modelPath = StagePart::GetModelPath(type);
        
        // パスを分解
        size_t lastSlash = modelPath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string directory = modelPath.substr(0, lastSlash);
            std::string filename = modelPath.substr(lastSlash + 1);
            
            // ファイルの存在を確認
            std::string fullPath = directory + "/" + filename;
            DWORD fileAttributes = GetFileAttributesA(fullPath.c_str());
            if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
                // ファイルが存在する場合のみ読み込み
                auto model = std::make_shared<Model>();
                model->Initialize(dxCommon_);
                model->LoadFromObj(directory, filename);
                partModels_[type] = model;
                
                OutputDebugStringA(("StageBuilder: Loaded model - " + fullPath + "\n").c_str());
            } else {
                OutputDebugStringA(("StageBuilder: Model file not found - " + fullPath + "\n").c_str());
            }
        }
    }
}

std::shared_ptr<StagePart> StageBuilder::CreatePart(StagePartType type) {
    std::shared_ptr<StagePart> part;

    // パーツタイプに応じて適切なクラスを作成
    switch (type) {
    case StagePartType::FLOOR_BASIC:
    case StagePartType::FLOOR_DAMAGED:
    case StagePartType::FLOOR_SPECIAL:
        part = std::make_shared<FloorPart>(type);
        break;
        
    case StagePartType::WALL_STRAIGHT:
    case StagePartType::WALL_CORNER:
    case StagePartType::WALL_T_JUNCTION:
    case StagePartType::WALL_CROSS:
    case StagePartType::WALL_DOOR:
        part = std::make_shared<WallPart>(type);
        break;
        
    default:
        part = std::make_shared<StagePart>(type);
        break;
    }

    if (part) {
        std::string modelPath = StagePart::GetModelPath(type);
        
        // ファイルの存在を確認
        size_t lastSlash = modelPath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string fullPath = modelPath;
            DWORD fileAttributes = GetFileAttributesA(fullPath.c_str());
            if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
                // ファイルが存在する場合のみ初期化
                part->Initialize(modelPath);
                
                // グラフィックスコンテキストが設定されていれば初期化
                if (dxCommon_ && spriteCommon_) {
                    part->InitializeGraphics(dxCommon_, spriteCommon_);
                }
            } else {
                OutputDebugStringA(("StageBuilder: CreatePart - Model file not found: " + fullPath + "\n").c_str());
                // モデルファイルが存在しない場合は、パーツを作成しない
                return nullptr;
            }
        }
    }

    return part;
}

std::shared_ptr<StagePart> StageBuilder::GetPartAt(int x, int y) const {
    if (grid_) {
        return grid_->GetPart(x, y);
    }
    return nullptr;
}

void StageBuilder::DrawDebugGrid() {
    // TODO: グリッドラインの描画実装
    // DirectXのラインプリミティブを使用してグリッドを描画
}

void StageBuilder::DrawConnectionIndicators() {
    // TODO: 接続可能な方向を示すインジケーターの描画
    // 各パーツの接続可能な方向に矢印や色を表示
}