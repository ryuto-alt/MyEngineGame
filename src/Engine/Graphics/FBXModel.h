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
    friend class FBXLoader;
public:
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector2 uv = Vector2(0.0f, 0.0f);
        Vector4 boneWeights = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        uint32_t boneIndices[4] = {0, 0, 0, 0};
    };

    struct Bone {
        std::string name;
        Matrix4x4 offsetMatrix = Matrix4x4::MakeIdentity();
        Matrix4x4 currentTransform = Matrix4x4::MakeIdentity();
        int parentIndex = -1;
    };

    struct AnimationKey {
        float time = 0.0f;
        Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
        Vector4 rotation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
    };

    struct AnimationChannel {
        std::string boneName;
        std::vector<AnimationKey> keys;
    };

    struct Animation {
        std::string name;
        float duration = 0.0f;
        float ticksPerSecond = 30.0f;
        std::unordered_map<std::string, AnimationChannel> channels;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialIndex = 0;
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
        D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    };

    struct Material {
        std::string name;
        std::string diffuseTexture;
        Vector4 diffuseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 ambientColor = Vector4(0.3f, 0.3f, 0.3f, 1.0f);
        Vector4 specularColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
        float shininess = 32.0f;
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
    const std::unordered_map<std::string, Animation>& GetAnimations() const { return animations; }
    bool IsAnimationPlaying() const { return isPlaying; }
    float GetAnimationTime() const { return currentTime; }
    
    // FBXLoader用のメソッド（FBXLoaderクラスからアクセス可能）
    void AddMesh(const Mesh& mesh) { meshes.push_back(mesh); }
    void AddMaterial(const Material& material) { materials.push_back(material); }
    void AddBone(const Bone& bone) { bones.push_back(bone); }
    void AddAnimation(const std::string& name, const Animation& animation) { animations[name] = animation; }
    int GetBoneIndex(const std::string& boneName) const;
    void SetBoneOffsetMatrix(int boneIndex, const Matrix4x4& matrix);

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