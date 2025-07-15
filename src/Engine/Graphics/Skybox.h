#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Camera.h"
#include "Mymath.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>

class DirectXCommon;
class SrvManager;
class TextureManager;

struct SkyboxVertex {
    Vector3 position;
};

class Skybox {
public:
    Skybox();
    ~Skybox();

    void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, TextureManager* textureManager);
    void LoadCubemap(const std::string& filePath);
    void Update();
    void Draw(Camera* camera);

    void SetScale(float scale) { scale_ = scale; }
    float GetScale() const { return scale_; }

private:
    void CreateVertexData();
    void CreateIndexData();
    void CreateMaterial();
    void CreateRootSignature();
    void CreateGraphicsPipelineState();

    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    TextureManager* textureManager_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

    SkyboxVertex* vertexData_ = nullptr;
    uint32_t* indexData_ = nullptr;
    Material* materialData_ = nullptr;
    TransformationMatrix* transformationMatrixData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

    std::string cubemapFilePath_;
    uint32_t cubemapSrvIndex_ = 0;
    float scale_ = 1000.0f;
    bool cubemapLoaded_ = false;

    static const uint32_t kNumVertices = 8;
    static const uint32_t kNumIndices = 36;
};