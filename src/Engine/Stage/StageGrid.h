#pragma once
#include <vector>
#include <memory>
#include "../Math/Vector3.h"

class StagePart;

class StageGrid {
public:
    static constexpr float GRID_SIZE = 4.0f;  // 4x4メートル単位

    StageGrid();
    ~StageGrid();

    void Initialize(int width, int height);

    bool CanPlacePart(int x, int y, StagePart* part) const;

    bool PlacePart(int x, int y, std::shared_ptr<StagePart> part);

    bool RemovePart(int x, int y);

    std::shared_ptr<StagePart> GetPart(int x, int y) const;

    void Clear();

    Vector3 GridToWorldPosition(int x, int y) const;

    void WorldToGridPosition(const Vector3& worldPos, int& x, int& y) const;

    bool IsValidPosition(int x, int y) const;

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

    const std::vector<std::vector<std::shared_ptr<StagePart>>>& GetGrid() const { return grid_; }

private:
    int width_;
    int height_;
    std::vector<std::vector<std::shared_ptr<StagePart>>> grid_;
};