#pragma once
#include "UnoEngine.h"

class LightManager {
public:
    LightManager();
    ~LightManager();

    void Initialize();
    void Update();
    void DrawImGui();

    // スポットライトをプレイヤー視点に追従させる
    void UpdateSpotLightFollowPlayer(const Vector3& playerPosition, const Vector3& cameraRotation);
    
    // ライトデータの取得
    const DirectionalLight& GetDirectionalLight() const { return directionalLight_; }
    const SpotLight& GetSpotLight() const { return spotLight_; }
    
    // デバッグ表示の切り替え
    void ToggleDebugDisplay() { showDebugWindow_ = !showDebugWindow_; }
    bool IsDebugDisplayShown() const { return showDebugWindow_; }
    
    // ライトの有効/無効
    bool IsDirectionalLightEnabled() const { return enableDirectionalLight_; }
    bool IsSpotLightEnabled() const { return enableSpotLight_; }

private:
    void NormalizeDirectionalLightDirection();
    void UpdateLightIntensity();
    
    // ライトデータ
    DirectionalLight directionalLight_;
    SpotLight spotLight_;
    
    // ライトの有効/無効フラグ
    bool enableDirectionalLight_ = true;
    bool enableSpotLight_ = true;
    
    // バックアップ用の強度値
    float dirLightIntensityBackup_ = 1.0f;
    float spotLightIntensityBackup_ = 2.0f;
    
    // デバッグ表示フラグ
    bool showDebugWindow_ = true;

    // 遅延追従用の変数
    Vector3 smoothedLightPosition_ = {0.0f, 5.0f, -2.0f};
    Vector3 smoothedLightDirection_ = {0.0f, -1.0f, 0.3f};
    float followSmoothness_ = 0.1f;  // 0.0〜1.0 (小さいほど遅延が大きい)
};