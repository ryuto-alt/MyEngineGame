#pragma once
#include "MazeGrid.h"
#include <random>
#include <stack>
#include <vector>

// 迷路生成アルゴリズムの種類
enum class MazeAlgorithm {
    RecursiveBacktracking,  // 再帰的バックトラッキング
    Prim,                  // プリムのアルゴリズム
    Kruskal,              // クラスカルのアルゴリズム
    RecursiveDivision,    // 再帰的分割
    BinaryTree,           // バイナリツリー
    DarkDeceptionStyle    // Dark Deception風（長い通路と少ない分岐）
};

// 迷路生成器
class MazeGenerator {
public:
    MazeGenerator();
    ~MazeGenerator() = default;

    // 迷路生成パラメータ
    struct GenerationParams {
        MazeAlgorithm algorithm = MazeAlgorithm::DarkDeceptionStyle;
        unsigned int seed = 0;           // 0の場合はランダムシード
        float wallDensity = 0.3f;        // 壁の密度 (0.0 ~ 1.0)
        float branchingFactor = 0.3f;    // 分岐の頻度 (0.0 ~ 1.0)
        float straightBias = 0.7f;       // 直線を優先する確率 (0.0 ~ 1.0)
        int minCorridorLength = 3;       // 最小通路長
        int maxCorridorLength = 8;       // 最大通路長
        bool createLoops = true;         // ループ（複数経路）を作成するか
        float loopProbability = 0.1f;    // ループ作成確率
        bool addRooms = true;            // 部屋を追加するか
        int roomCount = 3;               // 部屋の数
        int minRoomSize = 3;             // 最小部屋サイズ
        int maxRoomSize = 5;             // 最大部屋サイズ
    };

    // 迷路を生成
    void Generate(MazeGrid* grid, const GenerationParams& params = GenerationParams());

    // Dark Deception風の特殊エフェクト追加
    void AddDarkDeceptionEffects(MazeGrid* grid);

    // アイテムと敵の配置
    void PlaceItems(MazeGrid* grid, int itemCount);
    void PlaceEnemies(MazeGrid* grid, int enemyCount);

    // デッドエンド（行き止まり）の処理
    std::vector<Vector2> FindDeadEnds(MazeGrid* grid);
    void RemoveDeadEnds(MazeGrid* grid, int maxDeadEnds = 5);

private:
    std::mt19937 rng_;
    
    // 各アルゴリズムの実装
    void GenerateRecursiveBacktracking(MazeGrid* grid);
    void GeneratePrim(MazeGrid* grid);
    void GenerateDarkDeceptionStyle(MazeGrid* grid);
    
    // Dark Deceptionスタイルのヘルパー関数
    void CreateLongCorridor(MazeGrid* grid, int startX, int startY, Direction dir, int length);
    void CreateRoom(MazeGrid* grid, int centerX, int centerY, int width, int height);
    void CreateMainPath(MazeGrid* grid);
    void AddBranches(MazeGrid* grid, float branchingFactor);
    
    // ユーティリティ関数
    void ConnectCells(MazeGrid* grid, int x1, int y1, int x2, int y2);
    void CarvePath(MazeGrid* grid, int x, int y);
    std::vector<Direction> GetShuffledDirections();
    bool IsDeadEnd(MazeGrid* grid, int x, int y);
    int CountWalls(MazeGrid* grid, int x, int y);
    
    // ループ作成
    void CreateLoops(MazeGrid* grid, float probability);
    
    // 部屋生成
    void GenerateRooms(MazeGrid* grid, int roomCount, int minSize, int maxSize);
    bool CanPlaceRoom(MazeGrid* grid, int x, int y, int width, int height);
    
    // パス検証
    bool HasPath(MazeGrid* grid, int x1, int y1, int x2, int y2);
};