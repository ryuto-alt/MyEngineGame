#pragma once

#include "UnoEngine.h"
#include "../../Engine/Stage/StageBuilder.h"
#include "../../Engine/Stage/StagePart.h"

class StageEditorScene : public IScene {
public:
    // コンストラクタ・デストラクタ
    StageEditorScene();
    ~StageEditorScene() override;

    // ISceneの実装
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // ImGui描画
    void DrawImGui();
    void DrawPartPalette();
    void DrawGridView();
    void DrawToolbar();
    void DrawStageInfo();

    // 入力処理
    void HandleMouseInput();
    void HandleKeyboardInput();

    // グリッド座標計算
    void UpdateMouseGridPosition();

    // カメラ制御
    void UpdateCamera();

private:
    // 初期化済みフラグ
    bool initialized_ = false;

    // ステージビルダー
    StageBuilder* stageBuilder_ = nullptr;

    // カメラ設定
    bool isTopViewMode_ = true;
    float cameraDistance_ = 50.0f;
    float cameraAngle_ = 0.0f;
    float cameraPitch_ = 45.0f;

    // マウス状態
    int mouseGridX_ = 0;
    int mouseGridY_ = 0;
    bool isValidPlacement_ = false;

    // エディタ設定
    bool showGrid_ = true;
    bool showPalette_ = true;
    bool showInfo_ = true;

    // ステージサイズ設定
    int newStageWidth_ = 20;
    int newStageHeight_ = 20;

    // 選択中のパーツプレビュー
    std::unique_ptr<Object3d> previewObject_;
    std::unique_ptr<Model> previewModel_;

    // グリッド表示用
    std::vector<std::unique_ptr<Object3d>> gridLines_;

    // ファイル操作
    char saveFileName_[256] = "MyStage";
    std::vector<std::string> availableStages_;
    int selectedStageIndex_ = -1;
};