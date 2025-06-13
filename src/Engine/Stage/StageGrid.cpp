#include "StageGrid.h"
#include "StagePart.h"
#include <cmath>

StageGrid::StageGrid() : width_(0), height_(0) {
}

StageGrid::~StageGrid() {
}

void StageGrid::Initialize(int width, int height) {
    width_ = width;
    height_ = height;
    grid_.clear();
    grid_.resize(height, std::vector<std::shared_ptr<StagePart>>(width));
}

bool StageGrid::CanPlacePart(int x, int y, StagePart* part) const {
    if (!IsValidPosition(x, y)) {
        return false;
    }

    // 既にパーツが配置されている場合は配置不可
    if (grid_[y][x] != nullptr) {
        return false;
    }

    // TODO: パーツの接続ルールチェック
    // 隣接するパーツとの互換性をチェック

    return true;
}

bool StageGrid::PlacePart(int x, int y, std::shared_ptr<StagePart> part) {
    if (!CanPlacePart(x, y, part.get())) {
        return false;
    }

    grid_[y][x] = part;
    part->SetGridPosition(x, y);
    return true;
}

bool StageGrid::RemovePart(int x, int y) {
    if (!IsValidPosition(x, y)) {
        return false;
    }

    grid_[y][x] = nullptr;
    return true;
}

std::shared_ptr<StagePart> StageGrid::GetPart(int x, int y) const {
    if (!IsValidPosition(x, y)) {
        return nullptr;
    }
    return grid_[y][x];
}

void StageGrid::Clear() {
    for (auto& row : grid_) {
        for (auto& cell : row) {
            cell = nullptr;
        }
    }
}

Vector3 StageGrid::GridToWorldPosition(int x, int y) const {
    // グリッド座標をワールド座標に変換
    // グリッドの中心を原点とする
    float worldX = (x - width_ / 2.0f) * GRID_SIZE;
    float worldZ = (y - height_ / 2.0f) * GRID_SIZE;
    return Vector3(worldX, 0.0f, worldZ);
}

void StageGrid::WorldToGridPosition(const Vector3& worldPos, int& x, int& y) const {
    x = static_cast<int>(std::round(worldPos.x / GRID_SIZE + width_ / 2.0f));
    y = static_cast<int>(std::round(worldPos.z / GRID_SIZE + height_ / 2.0f));
}

bool StageGrid::IsValidPosition(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

