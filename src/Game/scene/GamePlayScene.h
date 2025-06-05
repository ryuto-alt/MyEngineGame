#pragma once
#include "UnoEngine.h"
#include "../StageEditor.h"
#include "Graphics/AnimatedObject3d.h"
#include "Graphics/FBXModel.h"
#include "Graphics/AnimatedRenderingPipeline.h"
#include <memory>

// GamePlayScene
class GamePlayScene : public IScene {
public:
    // コンストラクタ・デストラクタ
    GamePlayScene();
    ~GamePlayScene() override;

    // ISceneの実装
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

protected:
    // 初期化済みフラグ
    bool initialized_ = false;
    
    // ステージエディター
    std::unique_ptr<StageEditor> stageEditor_;
    
    // アニメーション関連
    std::unique_ptr<AnimatedRenderingPipeline> animatedPipeline_;
    std::shared_ptr<FBXModel> spiderModel_;
    std::unique_ptr<AnimatedObject3d> spiderObject_;
    
#ifdef _DEBUG
    bool isDebugMode_ = true;
#else
    bool isDebugMode_ = false;
#endif
};