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
/// ポータルエフェクトクラス
/// 異次元への扉のような渦巻くポータルエフェクト
/// </summary>
class PortalEffect {
public:
    /// <summary>
    /// ポータルのリング構造体
    /// </summary>
    struct PortalRing {
        std::unique_ptr<Ring> ring;
        std::unique_ptr<Object3d> object3d;
        float radius;
        float depth; // Z軸での位置
        float spiralSpeed;
        float spiralAngle;
        Vector4 baseColor;
        float distortionPhase;
        float fadeIntensity;
        int layerIndex;
    };

    PortalEffect();
    ~PortalEffect();

    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    void CreatePortal(const Vector3& position);
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
    void SetPortalType(int type); // 0: 青いポータル, 1: 赤いポータル
    bool IsActive() const { return isActive_; }

private:
    void CreatePortalRings();
    void UpdateRings(float deltaTime);
    void UpdateDistortion(float deltaTime);

    DirectXCommon* dxCommon_;
    SpriteCommon* spriteCommon_;
    Camera* camera_;

    std::vector<PortalRing> rings_;
    Vector3 position_;
    float scale_;
    float intensity_;
    float effectTime_;
    bool isVisible_;
    bool isActive_;
    int portalType_; // ポータルの種類

    // エフェクトパラメータ
    static constexpr int NUM_RINGS = 8;
    static constexpr float SPIRAL_SPEED = 2.0f;
    static constexpr float DEPTH_RANGE = 3.0f;
    static constexpr float DISTORTION_AMPLITUDE = 0.5f;
};
