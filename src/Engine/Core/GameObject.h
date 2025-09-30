#pragma once
#include "Vector3.h"
#include "Camera.h"
#include "Mymath.h"
#include <string>

class UnoEngine;

class GameObject {
public:
    virtual ~GameObject() = default;

    virtual void Initialize() {}
    virtual void Update() {}
    virtual void Draw() {}
    virtual void Finalize() {}

    void SetActive(bool active) { isActive_ = active; }
    bool IsActive() const { return isActive_; }

    void SetPosition(const Vector3& position) { position_ = position; }
    Vector3 GetPosition() const { return position_; }

    void SetRotation(const Vector3& rotation) { rotation_ = rotation; }
    Vector3 GetRotation() const { return rotation_; }

    void SetScale(const Vector3& scale) { scale_ = scale; }
    Vector3 GetScale() const { return scale_; }

    void SetName(const std::string& name) { name_ = name; }
    std::string GetName() const { return name_; }

    virtual void SetDirectionalLight(const DirectionalLight& light) { directionalLight_ = light; }
    virtual void SetSpotLight(const SpotLight& light) { spotLight_ = light; }

    void SetCamera(Camera* camera) { camera_ = camera; }
    Camera* GetCamera() const { return camera_; }

    void SetEngine(UnoEngine* engine) { engine_ = engine; }
    UnoEngine* GetEngine() const { return engine_; }

protected:
    bool isActive_ = true;
    std::string name_;

    Vector3 position_{0.0f, 0.0f, 0.0f};
    Vector3 rotation_{0.0f, 0.0f, 0.0f};
    Vector3 scale_{1.0f, 1.0f, 1.0f};

    Camera* camera_ = nullptr;
    UnoEngine* engine_ = nullptr;

    DirectionalLight directionalLight_;
    SpotLight spotLight_;
};