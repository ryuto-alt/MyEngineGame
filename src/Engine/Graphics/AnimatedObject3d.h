#pragma once
#include "Object3d.h"
#include "FBXModel.h"
#include <memory>

class AnimatedObject3d : public Object3d {
public:
    AnimatedObject3d();
    ~AnimatedObject3d();

    void Initialize();
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    void Update(float deltaTime);
    void Update();  // カメラを使用するUpdate
    void Draw();
    
    void SetFBXModel(std::shared_ptr<FBXModel> model) { fbxModel = model; }
    std::shared_ptr<FBXModel> GetFBXModel() const { return fbxModel; }
    
    void PlayAnimation(const std::string& animationName, bool loop = true);
    void StopAnimation();
    bool IsAnimationPlaying() const;

private:
    void CreateConstantBuffers();
    void UpdateConstantBuffers();

private:
    std::shared_ptr<FBXModel> fbxModel;
    Microsoft::WRL::ComPtr<ID3D12Resource> boneConstBuffer;
    
    struct BoneConstBufferData {
        Matrix4x4 bones[128];
    };
};