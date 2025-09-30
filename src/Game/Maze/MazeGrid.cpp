#include "MazeGrid.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

MazeGrid::MazeGrid(int width, int height, float cellSize)
    : width_(width)
    , height_(height)
    , cellSize_(cellSize)
    , startPos_(0, 0)
    , goalPos_(width - 1, height - 1) {
    
    // グリッドの初期化
    cells_.resize(height_);
    for (int y = 0; y < height_; ++y) {
        cells_[y].resize(width_);
        for (int x = 0; x < width_; ++x) {
            cells_[y][x] = std::make_unique<MazeCell>();
        }
    }
}

MazeCell* MazeGrid::GetCell(int x, int y) {
    if (!IsValidPosition(x, y)) {
        return nullptr;
    }
    return cells_[y][x].get();
}

const MazeCell* MazeGrid::GetCell(int x, int y) const {
    if (!IsValidPosition(x, y)) {
        return nullptr;
    }
    return cells_[y][x].get();
}

MazeCell* MazeGrid::GetCell(const Vector2& gridPos) {
    return GetCell(static_cast<int>(gridPos.x), static_cast<int>(gridPos.y));
}

Vector3 MazeGrid::GridToWorld(int x, int y) const {
    float worldX = x * cellSize_ - (width_ * cellSize_ * 0.5f) + cellSize_ * 0.5f;
    float worldZ = y * cellSize_ - (height_ * cellSize_ * 0.5f) + cellSize_ * 0.5f;
    return Vector3(worldX, 0.0f, worldZ);
}

Vector3 MazeGrid::GridToWorld(const Vector2& gridPos) const {
    return GridToWorld(static_cast<int>(gridPos.x), static_cast<int>(gridPos.y));
}

Vector2 MazeGrid::WorldToGrid(const Vector3& worldPos) const {
    float gridX = (worldPos.x + (width_ * cellSize_ * 0.5f) - cellSize_ * 0.5f) / cellSize_;
    float gridY = (worldPos.z + (height_ * cellSize_ * 0.5f) - cellSize_ * 0.5f) / cellSize_;
    return Vector2(gridX, gridY);
}

MazeCell* MazeGrid::GetNeighbor(int x, int y, Direction dir) {
    Vector2 offset = GetDirectionOffset(dir);
    int nx = x + static_cast<int>(offset.x);
    int ny = y + static_cast<int>(offset.y);
    return GetCell(nx, ny);
}

std::vector<std::pair<MazeCell*, Direction>> MazeGrid::GetNeighbors(int x, int y) {
    std::vector<std::pair<MazeCell*, Direction>> neighbors;
    
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        MazeCell* neighbor = GetNeighbor(x, y, dir);
        if (neighbor) {
            neighbors.push_back({neighbor, dir});
        }
    }
    
    return neighbors;
}

bool MazeGrid::IsValidPosition(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

bool MazeGrid::IsWalkable(int x, int y) const {
    const MazeCell* cell = GetCell(x, y);
    if (!cell) {
        return false;
    }
    
    MazeCellType type = cell->GetType();
    return type != MazeCellType::Wall && type != MazeCellType::Empty;
}

void MazeGrid::SetStartPosition(int x, int y) {
    if (IsValidPosition(x, y)) {
        // 前のスタート位置をリセット
        MazeCell* oldStart = GetCell(static_cast<int>(startPos_.x), static_cast<int>(startPos_.y));
        if (oldStart && oldStart->GetType() == MazeCellType::Start) {
            oldStart->SetType(MazeCellType::Floor);
        }
        
        // 新しいスタート位置を設定
        startPos_ = Vector2(static_cast<float>(x), static_cast<float>(y));
        MazeCell* newStart = GetCell(x, y);
        if (newStart) {
            newStart->SetType(MazeCellType::Start);
        }
    }
}

void MazeGrid::SetGoalPosition(int x, int y) {
    if (IsValidPosition(x, y)) {
        // 前のゴール位置をリセット
        MazeCell* oldGoal = GetCell(static_cast<int>(goalPos_.x), static_cast<int>(goalPos_.y));
        if (oldGoal && oldGoal->GetType() == MazeCellType::Goal) {
            oldGoal->SetType(MazeCellType::Floor);
        }
        
        // 新しいゴール位置を設定
        goalPos_ = Vector2(static_cast<float>(x), static_cast<float>(y));
        MazeCell* newGoal = GetCell(x, y);
        if (newGoal) {
            newGoal->SetType(MazeCellType::Goal);
        }
    }
}

void MazeGrid::SetDarkZone(int x1, int y1, int x2, int y2, float darkness) {
    for (int y = y1; y <= y2 && y < height_; ++y) {
        for (int x = x1; x <= x2 && x < width_; ++x) {
            MazeCell* cell = GetCell(x, y);
            if (cell) {
                cell->SetDarkness(darkness);
            }
        }
    }
}

void MazeGrid::SetFoggyZone(int x1, int y1, int x2, int y2, float fogDensity) {
    for (int y = y1; y <= y2 && y < height_; ++y) {
        for (int x = x1; x <= x2 && x < width_; ++x) {
            MazeCell* cell = GetCell(x, y);
            if (cell) {
                cell->SetFogDensity(fogDensity);
            }
        }
    }
}

void MazeGrid::Reset() {
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            cells_[y][x]->Reset();
        }
    }
    startPos_ = Vector2(0, 0);
    goalPos_ = Vector2(static_cast<float>(width_ - 1), static_cast<float>(height_ - 1));
}

MazeGrid::Statistics MazeGrid::GetStatistics() const {
    Statistics stats = {};
    stats.totalCells = width_ * height_;
    
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            const MazeCell* cell = GetCell(x, y);
            if (cell) {
                switch (cell->GetType()) {
                    case MazeCellType::Wall:
                        stats.wallCount++;
                        break;
                    case MazeCellType::Floor:
                    case MazeCellType::Start:
                    case MazeCellType::Goal:
                        stats.pathCount++;
                        break;
                    case MazeCellType::Item:
                        stats.itemCount++;
                        break;
                    case MazeCellType::Enemy:
                        stats.enemyCount++;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    return stats;
}

bool MazeGrid::Validate() const {
    // スタートとゴールが有効な位置にあるか確認
    if (!IsValidPosition(static_cast<int>(startPos_.x), static_cast<int>(startPos_.y)) ||
        !IsValidPosition(static_cast<int>(goalPos_.x), static_cast<int>(goalPos_.y))) {
        return false;
    }
    
    // 少なくとも1つの通路があるか確認
    Statistics stats = GetStatistics();
    return stats.pathCount > 0;
}

void MazeGrid::ForEachCell(std::function<void(int x, int y, MazeCell*)> callback) {
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            callback(x, y, cells_[y][x].get());
        }
    }
}

std::string MazeGrid::ExportToJSON() const {
    std::stringstream json;
    json << "{\n";
    json << "  \"width\": " << width_ << ",\n";
    json << "  \"height\": " << height_ << ",\n";
    json << "  \"cellSize\": " << cellSize_ << ",\n";
    json << "  \"startPos\": [" << startPos_.x << ", " << startPos_.y << "],\n";
    json << "  \"goalPos\": [" << goalPos_.x << ", " << goalPos_.y << "],\n";
    json << "  \"cells\": [\n";
    
    for (int y = 0; y < height_; ++y) {
        json << "    [";
        for (int x = 0; x < width_; ++x) {
            const MazeCell* cell = GetCell(x, y);
            json << static_cast<int>(cell->GetType());
            if (x < width_ - 1) json << ", ";
        }
        json << "]";
        if (y < height_ - 1) json << ",";
        json << "\n";
    }
    
    json << "  ]\n";
    json << "}\n";
    
    return json.str();
}

bool MazeGrid::ImportFromJSON(const std::string& json) {
    // JSONパース実装は後で追加
    // 現時点では簡易的な実装
    return false;
}

Direction MazeGrid::GetOppositeDirection(Direction dir) const {
    switch (dir) {
        case Direction::North: return Direction::South;
        case Direction::East: return Direction::West;
        case Direction::South: return Direction::North;
        case Direction::West: return Direction::East;
        default: return Direction::North;
    }
}

Vector2 MazeGrid::GetDirectionOffset(Direction dir) const {
    switch (dir) {
        case Direction::North: return Vector2(0, -1);
        case Direction::East: return Vector2(1, 0);
        case Direction::South: return Vector2(0, 1);
        case Direction::West: return Vector2(-1, 0);
        default: return Vector2(0, 0);
    }
}