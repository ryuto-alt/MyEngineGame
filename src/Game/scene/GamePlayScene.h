#pragma once
#include "UnoEngine.h"
#include "ParticleEmitter.h"
#include <memory>
#include <vector>

// パーティクルエフェクトのGamePlayScene
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

    // パーティクルエミッタ
    std::vector<std::unique_ptr<ParticleEmitter>> particleEmitters_;

    // エフェクト制御用
    float effectTimer_ = 0.0f;
    int currentEffect_ = 0;
    bool keyPressed_ = false;

    // エフェクト作成・制御関数
    void CreateEffectEmitters();
    void HandleEffectSwitching();
};