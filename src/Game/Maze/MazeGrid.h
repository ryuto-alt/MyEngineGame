#pragma once
#include "MazeCell.h"
#include "Vector2.h"
#include "Vector3.h"
#include <vector>
#include <memory>
#include <functional>
#include <string>

// 迷路グリッド構造
class MazeGrid {
public:
    MazeGrid(int width, int height, float cellSize = 2.0f);
    ~MazeGrid() = default;

    // グリッドサイズの取得
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    float GetCellSize() const { return cellSize_; }

    // セルの取得・設定
    MazeCell* GetCell(int x, int y);
    const MazeCell* GetCell(int x, int y) const;
    MazeCell* GetCell(const Vector2& gridPos);

    // 座標変換
    Vector3 GridToWorld(int x, int y) const;
    Vector3 GridToWorld(const Vector2& gridPos) const;
    Vector2 WorldToGrid(const Vector3& worldPos) const;

    // 隣接セルの取得
    MazeCell* GetNeighbor(int x, int y, Direction dir);
    std::vector<std::pair<MazeCell*, Direction>> GetNeighbors(int x, int y);

    // パス探索用
    bool IsValidPosition(int x, int y) const;
    bool IsWalkable(int x, int y) const;

    // スタート・ゴール地点の設定
    void SetStartPosition(int x, int y);
    void SetGoalPosition(int x, int y);
    Vector2 GetStartPosition() const { return startPos_; }
    Vector2 GetGoalPosition() const { return goalPos_; }

    // Dark Deceptionスタイルの特殊エリア設定
    void SetDarkZone(int x1, int y1, int x2, int y2, float darkness);
    void SetFoggyZone(int x1, int y1, int x2, int y2, float fogDensity);

    // 迷路のリセット
    void Reset();

    // 迷路の統計情報
    struct Statistics {
        int totalCells;
        int wallCount;
        int pathCount;
        int itemCount;
        int enemyCount;
    };
    Statistics GetStatistics() const;

    // 迷路の検証
    bool Validate() const;

    // セルごとに処理を実行
    void ForEachCell(std::function<void(int x, int y, MazeCell*)> callback);

    // JSON形式でのエクスポート/インポート
    std::string ExportToJSON() const;
    bool ImportFromJSON(const std::string& json);

private:
    int width_;
    int height_;
    float cellSize_;
    std::vector<std::vector<std::unique_ptr<MazeCell>>> cells_;
    Vector2 startPos_;
    Vector2 goalPos_;

    // ヘルパー関数
    Direction GetOppositeDirection(Direction dir) const;
    Vector2 GetDirectionOffset(Direction dir) const;
};