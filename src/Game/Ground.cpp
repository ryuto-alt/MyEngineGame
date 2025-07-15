#include "Ground.h"
#include <stdexcept>

Ground::Ground() {
}

Ground::~Ground() {
}

void Ground::Initialize(Camera* camera, DirectXCommon* dxCommon) {
    camera_ = camera;
    dxCommon_ = dxCommon;
    
    UnoEngine* engine = UnoEngine::GetInstance();
    
    try {
        model_ = std::make_unique<Model>();
        model_->Initialize(dxCommon_);
        model_->LoadFromGLB("Resources/Models/ground/ground.glb");
        
        object3d_ = engine->CreateObject3D();
        object3d_->SetModel(model_.get());
        object3d_->SetPosition(position_);
        
        // Blenderで設定されたスケール情報を使用
        const ModelData& modelData = model_->GetModelData();
        Vector3 blenderScale = modelData.rootTransform.scale;
        object3d_->SetScale(blenderScale);
        
        // デバッグ: 適用されたスケール値を出力
        char debugMsg[256];
        sprintf_s(debugMsg, "Ground: Applied Blender scale - X=%.3f, Y=%.3f, Z=%.3f\n",
                 blenderScale.x, blenderScale.y, blenderScale.z);
        OutputDebugStringA(debugMsg);
        
        object3d_->SetRotation(Vector3{0.0f, 0.0f, 0.0f});
        object3d_->SetEnableLighting(true);
        object3d_->SetCamera(camera_);
    } catch (const std::exception& e) {
        OutputDebugStringA(("Failed to load ground model: " + std::string(e.what()) + "\n").c_str());
    }
}

void Ground::Update() {
    if (!object3d_) return;
    object3d_->Update();
}

void Ground::Draw() {
    if (!object3d_) return;
    object3d_->Draw();
}

void Ground::Finalize() {
    if (object3d_) {
        object3d_.reset();
    }
    if (model_) {
        model_.reset();
    }
}

void Ground::SetDirectionalLight(const DirectionalLight& light) {
    if (object3d_) {
        object3d_->SetDirectionalLight(light);
    }
}

void Ground::SetSpotLight(const SpotLight& light) {
    if (object3d_) {
        object3d_->SetSpotLight(light);
    }
}

void Ground::SetEnvironmentTexture(const std::string& texturePath) {
    if (object3d_) {
        object3d_->SetEnvironmentTexture(texturePath);
    }
}

void Ground::SetEnableEnvironmentMap(bool enable) {
    if (object3d_) {
        object3d_->SetEnableEnvironmentMap(enable);
    }
}

bool Ground::GetEnableEnvironmentMap() const {
    if (object3d_) {
        return object3d_->GetEnableEnvironmentMap();
    }
    return false;
}

void Ground::SetPosition(const Vector3& position) {
    position_ = position;
    if (object3d_) {
        object3d_->SetPosition(position_);
    }
}

Vector3 Ground::GetPosition() const {
    return position_;
}