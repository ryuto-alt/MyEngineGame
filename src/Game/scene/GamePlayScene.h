#pragma once
#include "UnoEngine.h"
#include "../StageEditor.h"
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
    
#ifdef _DEBUG
    bool isDebugMode_ = true;
#else
    bool isDebugMode_ = false;
#endif
};