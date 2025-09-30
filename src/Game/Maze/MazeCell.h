#pragma once
#include <cstdint>
#include <array>

// 迷路のセルタイプ
enum class MazeCellType : uint8_t {
    Empty = 0,      // 空（通路）
    Wall = 1,       // 壁
    Floor = 2,      // 床
    Start = 3,      // スタート地点
    Goal = 4,       // ゴール地点
    Item = 5,       // アイテム配置可能位置
    Enemy = 6,      // 敵配置可能位置
    Portal = 7,     // ポータル（テレポート）
    Door = 8,       // ドア
    Key = 9         // 鍵
};

// 方向を表す列挙型
enum class Direction : uint8_t {
    North = 0,
    East = 1,
    South = 2,
    West = 3,
    Count = 4
};

// 迷路の個々のセル
class MazeCell {
public:
    MazeCell();
    ~MazeCell() = default;

    // セルタイプの設定・取得
    void SetType(MazeCellType type) { type_ = type; }
    MazeCellType GetType() const { return type_; }

    // 壁の設定・取得（各方向に壁があるかどうか）
    void SetWall(Direction dir, bool hasWall);
    bool HasWall(Direction dir) const;

    // 訪問済みフラグ（迷路生成時に使用）
    void SetVisited(bool visited) { visited_ = visited; }
    bool IsVisited() const { return visited_; }

    // 回転角度（度単位）
    void SetRotation(float rotation) { rotation_ = rotation; }
    float GetRotation() const { return rotation_; }

    // パーツIDの設定・取得（どのモデルを使うか）
    void SetPieceId(int id) { pieceId_ = id; }
    int GetPieceId() const { return pieceId_; }

    // 隣接するセルへの接続状態
    void SetConnected(Direction dir, bool connected);
    bool IsConnected(Direction dir) const;

    // Dark Deception風の特殊プロパティ
    void SetDarkness(float darkness) { darkness_ = darkness; }
    float GetDarkness() const { return darkness_; }

    void SetFogDensity(float density) { fogDensity_ = density; }
    float GetFogDensity() const { return fogDensity_; }

    // リセット
    void Reset();

private:
    MazeCellType type_;
    std::array<bool, 4> walls_;       // 各方向の壁の有無
    std::array<bool, 4> connections_; // 各方向への接続
    bool visited_;
    float rotation_;
    int pieceId_;
    float darkness_;    // 暗さレベル (0.0 ~ 1.0)
    float fogDensity_;  // 霧の濃度 (0.0 ~ 1.0)
};