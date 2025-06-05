#include "AnimatedRenderingPipeline.h"
#include "DirectXCommon.h"
#include <d3dcompiler.h>
#include <cassert>

#pragma comment(lib, "d3dcompiler.lib")

AnimatedRenderingPipeline::AnimatedRenderingPipeline() : dxCommon_(nullptr) {
}

AnimatedRenderingPipeline::~AnimatedRenderingPipeline() {
}

void AnimatedRenderingPipeline::Initialize(DirectXCommon* dxCommon) {
    dxCommon_ = dxCommon;
    CompileShaders();
    CreateRootSignature();
    CreatePipelineState();
    
    // 初期化結果の確認
    if (!vertexShaderBlob || !pixelShaderBlob) {
        OutputDebugStringA("AnimatedRenderingPipeline: Shader compilation failed\n");
    }
    if (!rootSignature) {
        OutputDebugStringA("AnimatedRenderingPipeline: Root signature creation failed\n");
    }
    if (!pipelineState) {
        OutputDebugStringA("AnimatedRenderingPipeline: Pipeline state creation failed\n");
    }
}

void AnimatedRenderingPipeline::CompileShaders() {
    ID3DBlob* errorBlob = nullptr;
    
    HRESULT hr = D3DCompileFromFile(
        L"Resources/shaders/AnimatedObject3d.VS.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vertexShaderBlob,
        &errorBlob
    );
    
    if (SUCCEEDED(hr)) {
        OutputDebugStringA("AnimatedRenderingPipeline: Vertex shader compiled successfully\n");
    }
    
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        assert(false);
    }
    
    hr = D3DCompileFromFile(
        L"Resources/shaders/Object3d.PS.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &pixelShaderBlob,
        &errorBlob
    );
    
    if (SUCCEEDED(hr)) {
        OutputDebugStringA("AnimatedRenderingPipeline: Pixel shader compiled successfully\n");
    }
    
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        assert(false);
    }
}

void AnimatedRenderingPipeline::CreateRootSignature() {
    if (!dxCommon_) return;
    
    auto device = dxCommon_->GetDevice();
    
    D3D12_ROOT_PARAMETER rootParameters[5] = {};
    
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 1;
    
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
    
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 1;
    
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[4].Descriptor.ShaderRegister = 4;
    
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumParameters = _countof(rootParameters);
    rootSignatureDesc.pStaticSamplers = staticSamplers;
    rootSignatureDesc.NumStaticSamplers = _countof(staticSamplers);
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    
    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    assert(SUCCEEDED(hr));
    
    hr = device->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    );
    assert(SUCCEEDED(hr));
    
    if (SUCCEEDED(hr)) {
        OutputDebugStringA("AnimatedRenderingPipeline: Root signature created successfully\n");
    }
    
    if (signatureBlob) signatureBlob->Release();
    if (errorBlob) errorBlob->Release();
}

void AnimatedRenderingPipeline::CreatePipelineState() {
    if (!dxCommon_) return;
    
    auto device = dxCommon_->GetDevice();
    
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // カリングを無効化
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    psoDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    
    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
    assert(SUCCEEDED(hr));
    
    if (SUCCEEDED(hr)) {
        OutputDebugStringA("AnimatedRenderingPipeline: Pipeline state created successfully\n");
    }
}

void AnimatedRenderingPipeline::Bind() {
    if (!dxCommon_) {
        OutputDebugStringA("AnimatedRenderingPipeline::Bind - dxCommon_ is null\n");
        return;
    }
    if (!rootSignature) {
        OutputDebugStringA("AnimatedRenderingPipeline::Bind - rootSignature is null\n");
        return;
    }
    if (!pipelineState) {
        OutputDebugStringA("AnimatedRenderingPipeline::Bind - pipelineState is null\n");
        return;
    }
    
    auto commandList = dxCommon_->GetCommandList();
    
    OutputDebugStringA("AnimatedRenderingPipeline::Bind - Setting root signature and pipeline state\n");
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pipelineState.Get());
    OutputDebugStringA("AnimatedRenderingPipeline::Bind - Successfully set\n");
}