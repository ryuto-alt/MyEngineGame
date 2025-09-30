#pragma once
#include "MazeGrid.h"
#include "Object3d.h"
#include "Model.h"
#include "Camera.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

// 迷路パーツの種類
enum class MazePieceType {
    Floor,
    WallStraight,
    WallCornerInner,
    WallCornerOuter,
    WallTJunction,
    WallCross,
    WallDeadEnd,
    Door,
    Portal,
    Light,
    Count
};

// 迷路レンダラー（効率的な描画のためインスタンシング対応）
class MazeRenderer {
public:
    MazeRenderer();
    ~MazeRenderer();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);

    // 迷路パーツモデルの読み込み
    void LoadMazePieces();

    // 迷路の構築（グリッドデータから3Dオブジェクトを生成）
    void BuildMaze(MazeGrid* grid);

    // 更新
    void Update(Camera* camera);

    // 描画
    void Draw();

    // Dark Deceptionエフェクトの更新
    void UpdateDarkDeceptionEffects(float deltaTime);

    // フォグ設定
    void SetFogEnabled(bool enabled) { fogEnabled_ = enabled; }
    void SetFogColor(const Vector4& color) { fogColor_ = color; }
    void SetFogDensity(float density) { fogDensity_ = density; }
    void SetFogStart(float start) { fogStart_ = start; }
    void SetFogEnd(float end) { fogEnd_ = end; }

    // ライティング設定
    void SetAmbientLight(const Vector3& color) { ambientLight_ = color; }
    void SetFlickerLightEnabled(bool enabled) { flickerLightEnabled_ = enabled; }

    // カリング設定
    void SetViewDistance(float distance) { viewDistance_ = distance; }
    void SetFrustumCullingEnabled(bool enabled) { frustumCullingEnabled_ = enabled; }

    // デバッグ表示
    void SetDebugDrawEnabled(bool enabled) { debugDrawEnabled_ = enabled; }

    // 統計情報
    struct RenderStatistics {
        int totalPieces;          // 総パーツ数
        int visiblePieces;        // 描画されたパーツ数
        int culledPieces;         // カリングされたパーツ数
        int instancedDrawCalls;   // インスタンス描画呼び出し数
        float frameTime;          // フレーム時間
    };
    const RenderStatistics& GetStatistics() const { return statistics_; }

private:
    DirectXCommon* dxCommon_;
    SpriteCommon* spriteCommon_;
    
    // パーツモデル（各種類ごとに1つのモデル）
    std::unordered_map<MazePieceType, std::unique_ptr<Model>> pieceModels_;
    
    // パーツインスタンス（同じモデルを共有する複数のObject3d）
    struct PieceInstance {
        std::unique_ptr<Object3d> object;
        MazePieceType type;
        Vector3 position;
        float rotation;
        bool isVisible;
        float darkness;     // セル固有の暗さ
        float fogDensity;   // セル固有の霧の濃度
    };
    std::vector<PieceInstance> pieceInstances_;
    
    // グリッド参照
    MazeGrid* currentGrid_;
    
    // フォグ設定
    bool fogEnabled_;
    Vector4 fogColor_;
    float fogDensity_;
    float fogStart_;
    float fogEnd_;
    
    // ライティング
    Vector3 ambientLight_;
    bool flickerLightEnabled_;
    float flickerTimer_;
    float flickerIntensity_;
    
    // カリング
    float viewDistance_;
    bool frustumCullingEnabled_;
    
    // デバッグ
    bool debugDrawEnabled_;
    
    // 統計
    RenderStatistics statistics_;
    
    // ヘルパー関数
    MazePieceType DeterminePieceType(MazeGrid* grid, int x, int y);
    float DeterminePieceRotation(MazeGrid* grid, int x, int y, MazePieceType type);
    void CreatePieceInstance(MazePieceType type, const Vector3& position, float rotation, float darkness, float fogDensity);
    bool IsInViewDistance(const Vector3& position, Camera* camera);
    bool IsInFrustum(const Vector3& position, Camera* camera);
    void UpdateFlickerLights(float deltaTime);
    void ApplyCellEffects(PieceInstance& instance);
    
    // パーツタイプ判定用のヘルパー関数
    int CountConnections(MazeGrid* grid, int x, int y);
    bool HasNorthConnection(MazeGrid* grid, int x, int y);
    bool HasEastConnection(MazeGrid* grid, int x, int y);
    bool HasSouthConnection(MazeGrid* grid, int x, int y);
    bool HasWestConnection(MazeGrid* grid, int x, int y);
};