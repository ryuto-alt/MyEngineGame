#include "Skybox.h"
#include "DirectXCommon.h"
#include "SRVManager.h"
#include "TextureManager.h"
#include "Mymath.h"
#include <cassert>

Skybox::Skybox() {}

Skybox::~Skybox() {}

void Skybox::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, TextureManager* textureManager) {
    assert(dxCommon);
    assert(srvManager);
    assert(textureManager);
    
    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
    textureManager_ = textureManager;

    CreateVertexData();
    CreateIndexData();
    CreateMaterial();
    CreateRootSignature();
    CreateGraphicsPipelineState();
}

void Skybox::CreateVertexData() {
    vertexResource_ = dxCommon_->CreateBufferResource(sizeof(SkyboxVertex) * kNumVertices);
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(SkyboxVertex) * kNumVertices;
    vertexBufferView_.StrideInBytes = sizeof(SkyboxVertex);

    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    // 立方体の8つの頂点（-1～1の範囲）
    vertexData_[0].position = { -1.0f,  1.0f, -1.0f }; // 左上前
    vertexData_[1].position = {  1.0f,  1.0f, -1.0f }; // 右上前
    vertexData_[2].position = { -1.0f, -1.0f, -1.0f }; // 左下前
    vertexData_[3].position = {  1.0f, -1.0f, -1.0f }; // 右下前
    vertexData_[4].position = { -1.0f,  1.0f,  1.0f }; // 左上後
    vertexData_[5].position = {  1.0f,  1.0f,  1.0f }; // 右上後
    vertexData_[6].position = { -1.0f, -1.0f,  1.0f }; // 左下後
    vertexData_[7].position = {  1.0f, -1.0f,  1.0f }; // 右下後
}

void Skybox::CreateIndexData() {
    indexResource_ = dxCommon_->CreateBufferResource(sizeof(uint32_t) * kNumIndices);
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * kNumIndices;

    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

    // 内側から見るためのインデックス（時計回り）
    uint32_t indices[kNumIndices] = {
        // 前面
        0, 2, 1, 1, 2, 3,
        // 右面  
        1, 3, 5, 5, 3, 7,
        // 後面
        5, 7, 4, 4, 7, 6,
        // 左面
        4, 6, 0, 0, 6, 2,
        // 上面
        4, 0, 5, 5, 0, 1,
        // 下面
        2, 6, 3, 3, 6, 7
    };

    for (uint32_t i = 0; i < kNumIndices; ++i) {
        indexData_[i] = indices[i];
    }
}

void Skybox::CreateMaterial() {
    materialResource_ = dxCommon_->CreateBufferResource(sizeof(Material));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = 0;

    transformationMatrixResource_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
}

void Skybox::CreateRootSignature() {
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].RegisterSpace = 0;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[3] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        assert(false);
    }

    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void Skybox::CreateGraphicsPipelineState() {
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Skybox.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Skybox.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // Skybox特有のDepthStencil設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 深度書き込み無効
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 最遠方として描画
    depthStencilDesc.StencilEnable = FALSE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.BlendState = blendDesc;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
    assert(SUCCEEDED(hr));
}

void Skybox::LoadCubemap(const std::string& filePath) {
    cubemapFilePath_ = filePath;
    
    // ファイルの存在確認
    DWORD fileAttributes = GetFileAttributesA(filePath.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        OutputDebugStringA(("Skybox: DDSファイルが見つかりません: " + filePath + "\n").c_str());
        cubemapLoaded_ = false;
        return;
    }
    
    // DDSファイルを読み込み
    bool loadResult = textureManager_->LoadTexture(filePath);
    if (!loadResult) {
        OutputDebugStringA(("Skybox: DDSファイルの読み込みに失敗しました: " + filePath + "\n").c_str());
        cubemapLoaded_ = false;
        return;
    }
    
    // CubemapのSRVインデックスを取得
    cubemapSrvIndex_ = textureManager_->GetSrvIndex(filePath);
    cubemapLoaded_ = true;
    
    OutputDebugStringA(("Skybox: Cubemap読み込み成功: " + filePath + ", SRVIndex: " + std::to_string(cubemapSrvIndex_) + "\n").c_str());
}

void Skybox::Update() {
    // 特に更新処理は不要
}

void Skybox::Draw(Camera* camera) {
    assert(camera);
    
    // Cubemapが正しく読み込まれていない場合は描画をスキップ
    if (!cubemapLoaded_) {
        OutputDebugStringA("Skybox: Cubemap読み込み未完了のため描画をスキップ\n");
        return;
    }

    OutputDebugStringA(("Skybox: 描画開始 - SRVIndex: " + std::to_string(cubemapSrvIndex_) + ", ファイル: " + cubemapFilePath_ + "\n").c_str());

    // ワールド行列（スケールのみ）
    Matrix4x4 worldMatrix = MakeScaleMatrix({ scale_, scale_, scale_ });
    // カメラの位置に移動（平行移動はカメラと一緒に移動）
    Vector3 cameraPos = camera->GetTranslate();
    worldMatrix.m[3][0] = cameraPos.x;
    worldMatrix.m[3][1] = cameraPos.y;
    worldMatrix.m[3][2] = cameraPos.z;

    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(camera->GetViewMatrix(), camera->GetProjectionMatrix()));
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;

    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
    dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
    dxCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, transformationMatrixResource_->GetGPUVirtualAddress());
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, materialResource_->GetGPUVirtualAddress());
    srvManager_->SetGraphicsRootDescriptorTable(2, cubemapSrvIndex_);

    dxCommon_->GetCommandList()->DrawIndexedInstanced(kNumIndices, 1, 0, 0, 0);
}