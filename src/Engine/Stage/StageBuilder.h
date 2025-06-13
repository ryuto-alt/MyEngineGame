#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include "StageGrid.h"
#include "StagePart.h"

class DirectXCommon;
class SpriteCommon;

class StageBuilder {
public:
    static StageBuilder& GetInstance();

    void Initialize();
    void SetGraphicsContext(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    void Finalize();

    void CreateNewStage(int width, int height);

    bool PlacePart(int x, int y, StagePartType type, int rotation = 0);

    bool RemovePart(int x, int y);

    bool RotatePart(int x, int y, int rotationDelta);

    void ClearStage();

    bool ValidateStageConnectivity() const;

    void Update();

    void Draw();

    // パーツライブラリ管理
    void PreloadPartModels();

    std::shared_ptr<StagePart> CreatePart(StagePartType type);

    // ゲッター
    StageGrid* GetGrid() { return grid_.get(); }
    const StageGrid* GetGrid() const { return grid_.get(); }
    
    std::shared_ptr<StagePart> GetPartAt(int x, int y) const;

    // ステージ情報
    std::string GetStageName() const { return stageName_; }
    void SetStageName(const std::string& name) { stageName_ = name; }

    // エディタ用の情報
    StagePartType GetSelectedPartType() const { return selectedPartType_; }
    void SetSelectedPartType(StagePartType type) { selectedPartType_ = type; }

    int GetSelectedRotation() const { return selectedRotation_; }
    void SetSelectedRotation(int rotation) { selectedRotation_ = rotation; }

    // グリッドスナップ
    bool IsGridSnapEnabled() const { return gridSnapEnabled_; }
    void SetGridSnapEnabled(bool enabled) { gridSnapEnabled_ = enabled; }

    // デバッグ表示
    bool IsDebugDrawEnabled() const { return debugDrawEnabled_; }
    void SetDebugDrawEnabled(bool enabled) { debugDrawEnabled_ = enabled; }

private:
    StageBuilder() = default;
    ~StageBuilder() = default;
    StageBuilder(const StageBuilder&) = delete;
    StageBuilder& operator=(const StageBuilder&) = delete;

    std::unique_ptr<StageGrid> grid_;
    std::string stageName_;

    // エディタ状態
    StagePartType selectedPartType_ = StagePartType::FLOOR_BASIC;
    int selectedRotation_ = 0;
    bool gridSnapEnabled_ = true;
    bool debugDrawEnabled_ = false;

    // パーツのプロトタイプ（モデルのキャッシュ用）
    std::unordered_map<StagePartType, std::shared_ptr<Model>> partModels_;

    // 配置されているすべてのパーツ
    std::vector<std::shared_ptr<StagePart>> allParts_;

    // グラフィックスコンテキスト
    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;

    void DrawDebugGrid();
    void DrawConnectionIndicators();
};