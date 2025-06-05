#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>

class AnimatedRenderingPipeline {
public:
    AnimatedRenderingPipeline();
    ~AnimatedRenderingPipeline();

    void Initialize(class DirectXCommon* dxCommon);
    void Bind();

private:
    void CreateRootSignature();
    void CreatePipelineState();
    void CompileShaders();

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
    
    class DirectXCommon* dxCommon_;
};