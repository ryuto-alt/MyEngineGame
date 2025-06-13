#pragma once
#include <string>
#include <memory>
#include <array>
#include "../Math/Vector3.h"

class Model;
class Object3d;
class DirectXCommon;
class SpriteCommon;

enum class StagePartType {
    FLOOR_BASIC,
    FLOOR_DAMAGED,
    FLOOR_SPECIAL,
    WALL_STRAIGHT,
    WALL_CORNER,
    WALL_T_JUNCTION,
    WALL_CROSS,
    WALL_DOOR,
    CEILING_BASIC,
    CEILING_LIGHT,
    STAIRS_UP,
    STAIRS_DOWN,
    PILLAR,
    DECORATION,
    MAX_TYPE
};

enum class Direction {
    NORTH = 0,  // +Z
    EAST = 1,   // +X
    SOUTH = 2,  // -Z
    WEST = 3    // -X
};

class StagePart {
public:
    StagePart(StagePartType type);
    virtual ~StagePart();

    void Initialize(const std::string& modelPath);
    void InitializeGraphics(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);

    void SetGridPosition(int x, int y);
    void SetWorldPosition(const Vector3& position);

    void SetRotation(int rotation);  // 0, 90, 180, 270度

    void Update();

    void Draw();

    // 接続可能な方向を取得
    bool CanConnect(Direction dir) const;

    // 隣接するパーツとの接続チェック
    bool IsCompatibleWith(const StagePart* other, Direction dir) const;

    // ゲッター
    StagePartType GetType() const { return type_; }
    int GetGridX() const { return gridX_; }
    int GetGridY() const { return gridY_; }
    int GetRotation() const { return rotation_; }
    const Vector3& GetWorldPosition() const { return worldPosition_; }
    Object3d* GetObject3d() const { return object3d_.get(); }

    // 各パーツタイプのモデルパスを取得
    static std::string GetModelPath(StagePartType type);

    // パーツタイプの文字列表現
    static std::string GetTypeName(StagePartType type);

protected:
    // 接続可能な方向のフラグ（北、東、南、西）
    std::array<bool, 4> connectableDirections_;

    // 回転を考慮した実際の接続方向を取得
    Direction GetRotatedDirection(Direction original) const;
    
    // 接続方向の更新
    void UpdateConnectionDirections();

private:
    StagePartType type_;
    int gridX_;
    int gridY_;
    Vector3 worldPosition_;
    int rotation_;  // 0, 90, 180, 270
    
    std::shared_ptr<Model> model_;
    std::unique_ptr<Object3d> object3d_;
    std::string modelPath_;  // モデルのパス保存用
};

// 具体的なパーツクラスの例
class FloorPart : public StagePart {
public:
    FloorPart(StagePartType type);
};

class WallPart : public StagePart {
public:
    WallPart(StagePartType type);
};