#include "MazeCell.h"

MazeCell::MazeCell()
    : type_(MazeCellType::Empty)
    , visited_(false)
    , rotation_(0.0f)
    , pieceId_(0)
    , darkness_(0.0f)
    , fogDensity_(0.0f) {
    
    // 初期状態では全方向に壁を設定
    for (int i = 0; i < 4; ++i) {
        walls_[i] = true;
        connections_[i] = false;
    }
}

void MazeCell::SetWall(Direction dir, bool hasWall) {
    walls_[static_cast<int>(dir)] = hasWall;
}

bool MazeCell::HasWall(Direction dir) const {
    return walls_[static_cast<int>(dir)];
}

void MazeCell::SetConnected(Direction dir, bool connected) {
    connections_[static_cast<int>(dir)] = connected;
    // 接続されている場合は壁を削除
    if (connected) {
        walls_[static_cast<int>(dir)] = false;
    }
}

bool MazeCell::IsConnected(Direction dir) const {
    return connections_[static_cast<int>(dir)];
}

void MazeCell::Reset() {
    type_ = MazeCellType::Empty;
    visited_ = false;
    rotation_ = 0.0f;
    pieceId_ = 0;
    darkness_ = 0.0f;
    fogDensity_ = 0.0f;
    
    for (int i = 0; i < 4; ++i) {
        walls_[i] = true;
        connections_[i] = false;
    }
}