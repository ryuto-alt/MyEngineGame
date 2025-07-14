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
        model_ = engine->LoadModel("Resources/Models/ground/ground.obj");
        if (model_) {
            object3d_ = engine->CreateObject3D();
            object3d_->SetModel(model_.get());
            object3d_->SetPosition(position_);
            object3d_->SetScale(Vector3{1.0f, 1.0f, 1.0f});
            object3d_->SetRotation(Vector3{0.0f, 0.0f, 0.0f});
            object3d_->SetEnableLighting(true);
            object3d_->SetCamera(camera_);
        }
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

void Ground::SetPosition(const Vector3& position) {
    position_ = position;
    if (object3d_) {
        object3d_->SetPosition(position_);
    }
}

Vector3 Ground::GetPosition() const {
    return position_;
}