#pragma once
#include "Ring.h"
#include "Object3d.h"
#include "Model.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Camera.h"
#include "Vector3.h"
#include "Vector4.h"
#include <memory>
#include <vector>

/// <summary>
/// 魔法陣エフェクトクラス
/// 複数の回転する円とルーンパターンで構成される派手な魔法エフェクト
/// </summary>
class MagicCircleEffect {
public:
    /// <summary>
    /// 魔法円のレイヤー構造体
    /// </summary>
    struct MagicLayer {
        std::unique_ptr<Ring> ring;
        std::unique_ptr<Object3d> object3d;
        float radius;
        float innerRatio;
        float rotationSpeed;
        float currentRotation;
        Vector4 color;
        float pulsePhase;
        float pulseSpeed;
        int divisions;
        bool reverse; // 逆回転フラグ
    };

    MagicCircleEffect();
    ~MagicCircleEffect();

    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    void CreateMagicCircle(const Vector3& position);
    void Update(float deltaTime);
    void Draw();

    void SetCamera(Camera* camera);
    void SetPosition(const Vector3& position);
    void SetScale(float scale);
    void SetIntensity(float intensity);
    void SetVisible(bool visible) { isVisible_ = visible; }
    bool IsVisible() const { return isVisible_; }

    // エフェクトの状態制御
    void StartEffect();
    void StopEffect();
    bool IsActive() const { return isActive_; }

private:
    void CreateLayers();
    void UpdateLayers(float deltaTime);
    void UpdateParticles(float deltaTime);

    DirectXCommon* dxCommon_;
    SpriteCommon* spriteCommon_;
    Camera* camera_;

    std::vector<MagicLayer> layers_;
    Vector3 position_;
    float scale_;
    float intensity_;
    float effectTime_;
    bool isVisible_;
    bool isActive_;

    // エフェクトパラメータ
    static constexpr int NUM_LAYERS = 5;
    static constexpr float BASE_ROTATION_SPEED = 1.0f;
    static constexpr float PULSE_AMPLITUDE = 0.3f;
};
