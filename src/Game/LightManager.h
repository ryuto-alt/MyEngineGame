#pragma once
#include "UnoEngine.h"

class LightManager {
public:
    LightManager();
    ~LightManager();

    void Initialize();
    void Update();
    void DrawImGui();
    
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
};