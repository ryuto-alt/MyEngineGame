#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include <d3d12.h>
#include <wrl.h>

class FBXModel {
public:
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector2 uv;
        Vector4 boneWeights;
        uint32_t boneIndices[4];
    };

    struct Bone {
        std::string name;
        Matrix4x4 offsetMatrix;
        Matrix4x4 currentTransform;
        int parentIndex;
    };

    struct AnimationKey {
        float time;
        Vector3 position;
        Vector4 rotation;
        Vector3 scale;
    };

    struct AnimationChannel {
        std::string boneName;
        std::vector<AnimationKey> keys;
    };

    struct Animation {
        std::string name;
        float duration;
        float ticksPerSecond;
        std::unordered_map<std::string, AnimationChannel> channels;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialIndex;
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
    };

    struct Material {
        std::string name;
        std::string diffuseTexture;
        Vector4 diffuseColor;
        Vector4 ambientColor;
        Vector4 specularColor;
        float shininess;
    };

public:
    FBXModel();
    ~FBXModel();

    void Initialize(class DirectXCommon* dxCommon);
    bool LoadFromFile(const std::string& filename);
    void Update(float deltaTime);
    void SetAnimation(const std::string& animationName);
    void PlayAnimation(bool loop = true);
    void StopAnimation();
    
    const std::vector<Mesh>& GetMeshes() const { return meshes; }
    const std::vector<Material>& GetMaterials() const { return materials; }
    const std::vector<Bone>& GetBones() const { return bones; }
    const std::vector<Matrix4x4>& GetBoneMatrices() const { return boneMatrices; }
    bool IsAnimationPlaying() const { return isPlaying; }
    float GetAnimationTime() const { return currentTime; }

private:
    bool ParseFBXBinary(const std::string& filename);
    bool ParseFBXAscii(const std::string& filename);
    void CreateBuffers();
    void UpdateAnimation(float deltaTime);
    void CalculateBoneTransforms();
    Matrix4x4 InterpolateTransform(const AnimationChannel& channel, float time);

private:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<Bone> bones;
    std::vector<Matrix4x4> boneMatrices;
    std::unordered_map<std::string, Animation> animations;
    
    std::string currentAnimation;
    float currentTime;
    float animationSpeed;
    bool isPlaying;
    bool isLooping;
    
    class DirectXCommon* dxCommon_;
};