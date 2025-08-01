#pragma once
#include "IScene.h"
#include "Camera.h"
#include "Object3d.h"
#include "Sprite.h"
#include "Skybox.h"
#include <memory>

class UnoEngine;

class DDSTestScene : public IScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

    void SetEngine(UnoEngine* engine) { engine_ = engine; }

private:
    UnoEngine* engine_ = nullptr;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Object3d> testObject_;
    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<Sprite> instructionSprite_;
    
    // カメラ制御用
    float cameraAngle_ = 0.0f;
    float cameraDistance_ = 20.0f;
    float cameraHeight_ = 5.0f;
    float rotationSpeed_ = 0.01f;
    
    // オブジェクト回転用
    float objectRotation_ = 0.0f;
};