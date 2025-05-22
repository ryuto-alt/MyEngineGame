#pragma once
#include "UnoEngine.h"
#include "ParticleEmitter.h"
#include "Particle3DDemo.h"
#include "EffectManager3D.h"
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

    // パーティクルエミッタ（2D）
    std::vector<std::unique_ptr<ParticleEmitter>> particleEmitters_;

    // 3Dパーティクルデモ
    std::unique_ptr<Particle3DDemo> particle3DDemo_;

    // エフェクト制御用
    float effectTimer_ = 0.0f;
    int currentEffect_ = 0;
    bool keyPressed_ = false;
    
    // 3Dエフェクト制御用
    Vector3 hitPosition3D_;
    float effect3DTimer_ = 0.0f;
    int current3DEffect_ = 0;
    bool key3DPressed_ = false;
    bool show3DEffects_ = true;  // 3Dエフェクト表示切り替え
    bool show2DEffects_ = true;  // 2Dエフェクト表示切り替え

    // エフェクト作成・制御関数
    void CreateEffectEmitters();
    void HandleEffectSwitching();
    void Handle3DEffectSwitching();  // 3Dエフェクト制御
    void TriggerRandom3DEffects();   // ランダムエフェクト発生
};