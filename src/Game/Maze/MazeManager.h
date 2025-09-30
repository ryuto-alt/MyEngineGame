#pragma once
#include "MazeGrid.h"
#include "MazeGenerator.h"
#include "MazeRenderer.h"
#include "Camera.h"
#include <memory>
#include <string>

// 迷路マネージャー（統合管理クラス）
class MazeManager {
public:
    MazeManager();
    ~MazeManager();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);

    // 迷路の生成
    void GenerateMaze(int width, int height, float cellSize = 2.0f);
    void GenerateMaze(const MazeGenerator::GenerationParams& params);

    // 迷路の読み込み/保存
    bool LoadMaze(const std::string& filepath);
    bool SaveMaze(const std::string& filepath);

    // 更新
    void Update(Camera* camera, float deltaTime);

    // 描画
    void Draw();

    // プレイヤー位置管理
    void SetPlayerPosition(const Vector3& position);
    Vector3 GetPlayerGridPosition() const;
    bool IsPlayerAtGoal() const;
    bool CanMoveTo(const Vector3& position) const;

    // アイテム/敵の管理
    void PlaceItems(int count);
    void PlaceEnemies(int count);
    bool CheckItemCollection(const Vector3& position);
    std::vector<Vector3> GetEnemyPositions() const;

    // Dark Deceptionエフェクト
    void EnableDarkDeceptionMode(bool enable);
    void SetHorrorIntensity(float intensity);
    void TriggerJumpScare(const Vector3& position);

    // 迷路情報の取得
    MazeGrid* GetGrid() { return mazeGrid_.get(); }
    const MazeGrid* GetGrid() const { return mazeGrid_.get(); }
    MazeRenderer* GetRenderer() { return mazeRenderer_.get(); }

    // デバッグ機能
    void ShowDebugInfo(bool show) { showDebugInfo_ = show; }
    void RegenerateMaze(); // 迷路を再生成

    // ImGui用のデバッグウィンドウ
    void DrawImGuiDebugWindow();

private:
    std::unique_ptr<MazeGrid> mazeGrid_;
    std::unique_ptr<MazeGenerator> mazeGenerator_;
    std::unique_ptr<MazeRenderer> mazeRenderer_;
    
    // 現在の設定
    MazeGenerator::GenerationParams currentParams_;
    int currentWidth_;
    int currentHeight_;
    float currentCellSize_;
    
    // プレイヤー状態
    Vector3 playerPosition_;
    Vector2 playerGridPos_;
    
    // アイテム/敵の位置
    std::vector<Vector3> itemPositions_;
    std::vector<Vector3> enemyPositions_;
    std::vector<bool> itemCollected_;
    
    // Dark Deceptionモード
    bool darkDeceptionMode_;
    float horrorIntensity_;
    float jumpScareTimer_;
    Vector3 jumpScarePosition_;
    
    // デバッグ
    bool showDebugInfo_;
    
    // DirectX参照
    DirectXCommon* dxCommon_;
    SpriteCommon* spriteCommon_;
    
    // ヘルパー関数
    void UpdatePlayerGridPosition();
    void UpdateEnemyAI(float deltaTime);
    void CheckCollisions();
};