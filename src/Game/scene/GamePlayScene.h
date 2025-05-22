#pragma once
#include "IScene.h"
#include "UnoEngine.h"
#include "CircleEffect.h"
#include "RingManager.h"
#include "Object3d.h"
#include "Input.h"
#include "imgui.h"
#include "Vector3.h"
#include "Vector4.h"
#include <memory>
#include <vector>

// 円形エフェクトのGamePlayScene
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

    // エフェクト切り替え関連
    float effectTimer_ = 0.0f;       // エフェクト制御タイマー
    int currentEffect_ = 0;          // 現在のエフェクトタイプ
    bool keyPressed_ = false;        // キー押下状態
    
    // 円形エフェクト関連
    std::vector<std::unique_ptr<CircleEffect>> circleEffects_;
    float animationTime_ = 0.0f;     // アニメーション用タイマー
    
    // エフェクト関数
    void CreateCircleEffects();      // 円形エフェクト作成
    void UpdateCircleEffects();      // 円形エフェクト更新
    void HandleEffectSwitching();    // エフェクト切り替え
    void ShowEffectUI();             // UI表示
    
    // エフェクト設定構造体
    struct EffectSettings {
        Vector3 position;
        Vector3 rotation;
        Vector3 scale;
        Vector4 color;
        float outerRadius;
        float innerRadius;
        uint32_t divisions;
        float uvScrollSpeedU;
        float uvScrollSpeedV;
        bool enableAnimation;
        std::string name;
        std::string description;
    };
    
    std::vector<EffectSettings> effectSettings_;
};