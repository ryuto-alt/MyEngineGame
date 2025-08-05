#include "LightManager.h"
#include "Camera.h"
#include <cmath>

LightManager::LightManager() {
}

LightManager::~LightManager() {
}

void LightManager::Initialize() {
    // ディレクショナルライトの初期設定（非常に弱く設定）
    directionalLight_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLight_.direction = { 0.0f, -1.0f, 0.5f };
    directionalLight_.intensity = 0.05f;  // 1.0fから0.05fに減らす（ほぼ効果なし）
    
    // スポットライトの初期設定
    spotLight_.color = { 1.0f, 0.9f, 0.8f, 1.0f };
    spotLight_.position = { 0.0f, 5.0f, -2.0f };
    spotLight_.intensity = 2.0f;
    spotLight_.direction = { 0.0f, -1.0f, 0.3f };
    spotLight_.innerCone = cosf(12.0f * 3.14159265f / 180.0f);
    spotLight_.attenuation = { 1.0f, 0.09f, 0.032f };
    spotLight_.outerCone = cosf(20.0f * 3.14159265f / 180.0f);
    
    // 初期値をバックアップ
    dirLightIntensityBackup_ = directionalLight_.intensity;
    spotLightIntensityBackup_ = spotLight_.intensity;
    
    // デフォルトでディレクショナルライトを無効化（スポットライトのみ使用）
    enableDirectionalLight_ = false;
}

void LightManager::Update() {
    UpdateLightIntensity();
}

void LightManager::DrawImGui() {
    if (!showDebugWindow_) return;
    
    ImGui::Begin("Lighting Settings (F \u30ad\u30fc\u3067\u8868\u793a\u5207\u66ff)");
    
    // ディレクショナルライト設定
    ImGui::Separator();
    ImGui::Text("Directional Light");
    ImGui::Checkbox("Enable Directional Light", &enableDirectionalLight_);
    if (enableDirectionalLight_) {
        ImGui::ColorEdit3("Dir Light Color", &directionalLight_.color.x);
        ImGui::SliderFloat3("Dir Light Direction", &directionalLight_.direction.x, -1.0f, 1.0f);
        ImGui::SliderFloat("Dir Light Intensity", &directionalLight_.intensity, 0.0f, 3.0f);
        
        NormalizeDirectionalLightDirection();
    }
    
    // スポットライト設定
    ImGui::Separator();
    ImGui::Text("Spot Light");
    ImGui::Checkbox("Enable Spot Light", &enableSpotLight_);
    if (enableSpotLight_) {
        ImGui::ColorEdit3("Spot Light Color", &spotLight_.color.x);
        ImGui::DragFloat3("Spot Light Position", &spotLight_.position.x, 0.1f);
        ImGui::SliderFloat3("Spot Light Direction", &spotLight_.direction.x, -1.0f, 1.0f);
        ImGui::SliderFloat("Spot Light Intensity", &spotLight_.intensity, 0.0f, 5.0f);
        
        // コーン角度（度数で表示）
        float innerAngle = acosf(spotLight_.innerCone) * 180.0f / 3.14159265f;
        float outerAngle = acosf(spotLight_.outerCone) * 180.0f / 3.14159265f;
        
        if (ImGui::SliderFloat("Inner Cone Angle", &innerAngle, 0.0f, 90.0f)) {
            spotLight_.innerCone = cosf(innerAngle * 3.14159265f / 180.0f);
        }
        if (ImGui::SliderFloat("Outer Cone Angle", &outerAngle, 0.0f, 90.0f)) {
            spotLight_.outerCone = cosf(outerAngle * 3.14159265f / 180.0f);
        }
        
        // 減衰パラメータ
        ImGui::Text("Attenuation");
        ImGui::SliderFloat("Constant", &spotLight_.attenuation.x, 0.0f, 2.0f);
        ImGui::SliderFloat("Linear", &spotLight_.attenuation.y, 0.0f, 0.5f);
        ImGui::SliderFloat("Quadratic", &spotLight_.attenuation.z, 0.0f, 0.1f);
    }
    
    // 現在のライト値の表示
    ImGui::Separator();
    ImGui::Text("\u73fe\u5728\u306e\u30e9\u30a4\u30c8\u5024:");
    ImGui::Text("Directional Light:");
    ImGui::Text("  \u5f37\u5ea6: %.2f", directionalLight_.intensity);
    ImGui::Text("  \u65b9\u5411: (%.2f, %.2f, %.2f)", 
        directionalLight_.direction.x, directionalLight_.direction.y, directionalLight_.direction.z);
    ImGui::Text("Spot Light:");
    ImGui::Text("  \u5f37\u5ea6: %.2f", spotLight_.intensity);
    ImGui::Text("  \u4f4d\u7f6e: (%.2f, %.2f, %.2f)", 
        spotLight_.position.x, spotLight_.position.y, spotLight_.position.z);
    
    ImGui::End();
}

void LightManager::NormalizeDirectionalLightDirection() {
    float dirLength = sqrtf(
        directionalLight_.direction.x * directionalLight_.direction.x +
        directionalLight_.direction.y * directionalLight_.direction.y +
        directionalLight_.direction.z * directionalLight_.direction.z
    );
    
    if (dirLength > 0.001f) {
        directionalLight_.direction.x /= dirLength;
        directionalLight_.direction.y /= dirLength;
        directionalLight_.direction.z /= dirLength;
    }
}

void LightManager::UpdateLightIntensity() {
    // ディレクショナルライトの強度管理
    if (!enableDirectionalLight_) {
        if (directionalLight_.intensity > 0.0f) {
            dirLightIntensityBackup_ = directionalLight_.intensity;
        }
        directionalLight_.intensity = 0.0f;
    } else if (directionalLight_.intensity == 0.0f) {
        directionalLight_.intensity = dirLightIntensityBackup_;
    }
    
    // スポットライトの強度管理
    if (!enableSpotLight_) {
        if (spotLight_.intensity > 0.0f) {
            spotLightIntensityBackup_ = spotLight_.intensity;
        }
        spotLight_.intensity = 0.0f;
    } else if (spotLight_.intensity == 0.0f) {
        spotLight_.intensity = spotLightIntensityBackup_;
    }
}

void LightManager::UpdateSpotLightPosition(const Vector3& playerPos, float playerRotationY) {
    if (!followPlayer_) return;
    
    // プレイヤーの位置から少し上にスポットライトを配置
    spotLight_.position = playerPos;
    spotLight_.position.y += 2.5f; // プレイヤーの頭上2.5ユニット
    
    // プレイヤーの前方にライトを向ける
    float cosY = cosf(playerRotationY);
    float sinY = sinf(playerRotationY);
    
    // 前方向を計算（プレイヤーの向いている方向）
    spotLight_.direction.x = sinY;
    spotLight_.direction.y = -0.8f; // 少し下向き
    spotLight_.direction.z = cosY;
    
    // 方向ベクトルを正規化
    float dirLength = sqrtf(
        spotLight_.direction.x * spotLight_.direction.x +
        spotLight_.direction.y * spotLight_.direction.y +
        spotLight_.direction.z * spotLight_.direction.z
    );
    
    if (dirLength > 0.001f) {
        spotLight_.direction.x /= dirLength;
        spotLight_.direction.y /= dirLength;
        spotLight_.direction.z /= dirLength;
    }
}

void LightManager::UpdateSpotLightForFirstPerson(Camera* camera) {
    if (!camera || !followPlayer_) return;
    
    // カメラの位置をスポットライトの位置に設定
    Vector3 cameraPos = camera->GetTranslate();
    spotLight_.position = cameraPos;
    
    // カメラの前方ベクトルを取得してライトの方向に設定
    Vector3 forward = camera->GetForwardVector();
    spotLight_.direction = forward;
    
    // 方向ベクトルを正規化（GetForwardVectorは既に正規化されているはずだが念のため）
    float dirLength = sqrtf(
        spotLight_.direction.x * spotLight_.direction.x +
        spotLight_.direction.y * spotLight_.direction.y +
        spotLight_.direction.z * spotLight_.direction.z
    );
    
    if (dirLength > 0.001f) {
        spotLight_.direction.x /= dirLength;
        spotLight_.direction.y /= dirLength;
        spotLight_.direction.z /= dirLength;
    }
    
    // 一人称視点用のスポットライト設定
    spotLight_.innerCone = cosf(15.0f * 3.14159265f / 180.0f);  // 内側15度
    spotLight_.outerCone = cosf(25.0f * 3.14159265f / 180.0f);  // 外側25度
    spotLight_.intensity = 3.0f;  // 明るめに設定
}