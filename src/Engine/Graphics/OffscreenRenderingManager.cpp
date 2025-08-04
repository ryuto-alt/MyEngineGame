#include "OffscreenRenderingManager.h"
#include "DirectXCommon.h"
#include "SRVManager.h"
#include "d3dx12.h"
#include <cassert>

OffscreenRenderingManager::~OffscreenRenderingManager() {
    // シェーダーブロブのリソースを解放
    if (copyImageVertexShader_) {
        copyImageVertexShader_->Release();
        copyImageVertexShader_ = nullptr;
    }
    if (copyImagePixelShader_) {
        copyImagePixelShader_->Release();
        copyImagePixelShader_ = nullptr;
    }
    if (grayscalePixelShader_) {
        grayscalePixelShader_->Release();
        grayscalePixelShader_ = nullptr;
    }
    if (vignettePixelShader_) {
        vignettePixelShader_->Release();
        vignettePixelShader_ = nullptr;
    }
}

void OffscreenRenderingManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,
    uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
    
    assert(dxCommon);
    assert(srvManager);

    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
    textureWidth_ = width;
    textureHeight_ = height;

    // RenderTextureの作成
    renderTexture_ = std::make_unique<RenderTexture>();
    
    if (!renderTexture_) {
        return;
    }
    
    renderTexture_->Initialize(dxCommon_, srvManager_, width, height, format, clearColor);

    // シェーダーコンパイル
    CompileShaders();

    // RootSignatureの作成
    CreateRootSignature();

    // 各種パイプラインステートオブジェクトを作成
    CreateCopyImagePSO();
    CreateGrayscalePSO();
    CreateVignettePSO();
}

void OffscreenRenderingManager::CompileShaders() {
    // フルスクリーン処理用の共通VertexShaderをコンパイル
    copyImageVertexShader_ = dxCommon_->CompileShader(L"Resources/shaders/Fullscreen.VS.hlsl", L"vs_6_0");
    if (!copyImageVertexShader_) {
        OutputDebugStringA("ERROR: Failed to compile Fullscreen.VS.hlsl\n");
        assert(false);
    }
    
    // CopyImage用PixelShaderをコンパイル
    copyImagePixelShader_ = dxCommon_->CompileShader(L"Resources/shaders/CopyImage.PS.hlsl", L"ps_6_0");
    if (!copyImagePixelShader_) {
        OutputDebugStringA("ERROR: Failed to compile CopyImage.PS.hlsl\n");
        assert(false);
    }
    
    // Grayscale用PixelShaderをコンパイル
    grayscalePixelShader_ = dxCommon_->CompileShader(L"Resources/shaders/Grayscale.PS.hlsl", L"ps_6_0");
    if (!grayscalePixelShader_) {
        OutputDebugStringA("ERROR: Failed to compile Grayscale.PS.hlsl\n");
        assert(false);
    }
    
    // Vignette用PixelShaderをコンパイル
    vignettePixelShader_ = dxCommon_->CompileShader(L"Resources/shaders/Vignette.PS.hlsl", L"ps_6_0");
    if (!vignettePixelShader_) {
        OutputDebugStringA("ERROR: Failed to compile Vignette.PS.hlsl\n");
        assert(false);
    }
}

void OffscreenRenderingManager::CreateRootSignature() {
    // 資料に基づく実装：RootParameterはTextureとSamplerが使えるように設定
    
    // Descriptor Range
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Root Parameter
    D3D12_ROOT_PARAMETER rootParameters[2] = {};
    
    // テクスチャ用
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    // サンプラー用
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = nullptr; // サンプラー用は後で設定

    // Sampler
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumParameters = 1; // テクスチャのみ使用
    rootSignatureDesc.pStaticSamplers = staticSamplers;
    rootSignatureDesc.NumStaticSamplers = _countof(staticSamplers);

    // シリアライズしてルートシグネチャ作成
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    
    if (FAILED(hr)) {
        if (errorBlob) {
            // エラー情報をログ出力
            OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
        }
        assert(false);
    }

    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&copyImageRootSignature_));
    assert(SUCCEEDED(hr));
}

void OffscreenRenderingManager::CreateCopyImagePSO() {
    // 資料に基づく実装
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    // RootSignature
    pipelineStateDesc.pRootSignature = copyImageRootSignature_.Get();

    // InputLayoutの設定（資料より：頂点には何もデータを入力しないので、InputLayoutはnullptr、NumElements = 0）
    pipelineStateDesc.InputLayout.pInputElementDescs = nullptr;
    pipelineStateDesc.InputLayout.NumElements = 0;

    // Shader（IDxcBlobから手動でD3D12_SHADER_BYTECODEを構築）
    pipelineStateDesc.VS.pShaderBytecode = copyImageVertexShader_->GetBufferPointer();
    pipelineStateDesc.VS.BytecodeLength = copyImageVertexShader_->GetBufferSize();
    pipelineStateDesc.PS.pShaderBytecode = copyImagePixelShader_->GetBufferPointer();
    pipelineStateDesc.PS.BytecodeLength = copyImagePixelShader_->GetBufferSize();

    // BlendState
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // RasterizerState
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    // DepthStencilStateの設定（資料より：全画面に対してなに処理をほどこしたいだけだから、比較も書き込みも必要ないのでDepth自体不要）
    pipelineStateDesc.DepthStencilState.DepthEnable = false;
    pipelineStateDesc.DepthStencilState.StencilEnable = false;

    // DSVFormat
    pipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN; // Depthを使わないのでUNKNOWN

    // PrimitiveTopology
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // RTVFormat
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    // Sample
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // PipelineState作成
    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc,
        IID_PPV_ARGS(&copyImagePipelineState_));
    assert(SUCCEEDED(hr));
}

void OffscreenRenderingManager::CreateGrayscalePSO() {
    // グレースケール用のパイプラインステート作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    // RootSignature（共通のものを使用）
    pipelineStateDesc.pRootSignature = copyImageRootSignature_.Get();

    // InputLayout（共通：頂点データなし）
    pipelineStateDesc.InputLayout.pInputElementDescs = nullptr;
    pipelineStateDesc.InputLayout.NumElements = 0;

    // Shader（VertexShaderは共通、PixelShaderはグレースケール用）
    pipelineStateDesc.VS.pShaderBytecode = copyImageVertexShader_->GetBufferPointer();
    pipelineStateDesc.VS.BytecodeLength = copyImageVertexShader_->GetBufferSize();
    pipelineStateDesc.PS.pShaderBytecode = grayscalePixelShader_->GetBufferPointer();
    pipelineStateDesc.PS.BytecodeLength = grayscalePixelShader_->GetBufferSize();

    // BlendState（共通）
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // RasterizerState（共通）
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    // DepthStencilState（共通：無効）
    pipelineStateDesc.DepthStencilState.DepthEnable = false;
    pipelineStateDesc.DepthStencilState.StencilEnable = false;

    // DSVFormat（共通）
    pipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

    // PrimitiveTopology（共通）
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // RTVFormat（共通）
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    // Sample（共通）
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // PipelineState作成
    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc,
        IID_PPV_ARGS(&grayscalePipelineState_));
    assert(SUCCEEDED(hr));
}

void OffscreenRenderingManager::CreateVignettePSO() {
    // Vignetting用のパイプラインステート作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    // RootSignature（共通のものを使用）
    pipelineStateDesc.pRootSignature = copyImageRootSignature_.Get();

    // InputLayout（共通：頂点データなし）
    pipelineStateDesc.InputLayout.pInputElementDescs = nullptr;
    pipelineStateDesc.InputLayout.NumElements = 0;

    // Shader（VertexShaderは共通、PixelShaderはVignette用）
    pipelineStateDesc.VS.pShaderBytecode = copyImageVertexShader_->GetBufferPointer();
    pipelineStateDesc.VS.BytecodeLength = copyImageVertexShader_->GetBufferSize();
    pipelineStateDesc.PS.pShaderBytecode = vignettePixelShader_->GetBufferPointer();
    pipelineStateDesc.PS.BytecodeLength = vignettePixelShader_->GetBufferSize();

    // BlendState（共通）
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // RasterizerState（共通）
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    // DepthStencilState（共通：無効）
    pipelineStateDesc.DepthStencilState.DepthEnable = false;
    pipelineStateDesc.DepthStencilState.StencilEnable = false;

    // DSVFormat（共通）
    pipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

    // PrimitiveTopology（共通）
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // RTVFormat（共通）
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    // Sample（共通）
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // PipelineState作成
    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc,
        IID_PPV_ARGS(&vignettePipelineState_));
    assert(SUCCEEDED(hr));
}

void OffscreenRenderingManager::BeginRenderToTexture() {
    // リソースの有効性チェック
    if (!renderTexture_) {
        OutputDebugStringA("OffscreenRenderingManager::BeginRenderToTexture - ERROR: renderTexture_ is null!\n");
        return;
    }
    
    if (!dxCommon_) {
        OutputDebugStringA("OffscreenRenderingManager::BeginRenderToTexture - ERROR: dxCommon_ is null!\n");
        return;
    }
    
    // RenderTextureをレンダーターゲットとして設定
    renderTexture_->BeginRenderTarget();
}

void OffscreenRenderingManager::EndRenderToTexture() {
    if (!renderTexture_) {
        OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - ERROR: renderTexture_ is null!\n");
        return;
    }
    
    if (!dxCommon_) {
        OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - ERROR: dxCommon_ is null!\n");
        return;
    }
    
    // RenderTextureのレンダーターゲット使用を終了
    renderTexture_->EndRenderTarget();
    
    // 【重要修正】CommandKick()を呼ばずにリソース状態遷移のみ実行
    // 同一コマンドリスト内でRenderTexture → SwapChain描画を継続
}

void OffscreenRenderingManager::CopyToSwapChain() {
    if (!dxCommon_) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: dxCommon_ is null!\n");
        return;
    }
    
    if (!renderTexture_) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: renderTexture_ is null!\n");
        return;
    }
    
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
    if (!commandList) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: commandList is null!\n");
        return;
    }
    
    // 【重要修正】RenderTextureがSHADER_RESOURCE状態になっていることを確実にする
    renderTexture_->EnsureShaderResourceState();
    
    // 【重要修正】UnoEngine::Begin()で既にSwapChainはRENDER_TARGET状態のため、リソースバリアは不要
    if (!dxCommon_->GetSwapChain()) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: SwapChain is null!\n");
        return;
    }
    
    UINT backBufferIndex = dxCommon_->GetSwapChain()->GetCurrentBackBufferIndex();
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource = dxCommon_->GetSwapChainResource(backBufferIndex);
    
    if (!swapChainResource) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: SwapChain resource is null!\n");
        return;
    }
    
    // 【重要】SwapChainを明示的にRenderTargetとして設定（RenderTextureのアクティブ状態を解除）
    D3D12_CPU_DESCRIPTOR_HANDLE swapChainRtvHandle = dxCommon_->GetRTVCPUDescriptorHandle(backBufferIndex);
    commandList->OMSetRenderTargets(1, &swapChainRtvHandle, FALSE, nullptr);

    // ビューポートとシザー矩形の明示的設定（DirectXCommonの設定を確実に適用）
    D3D12_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(textureWidth_);
    viewport.Height = static_cast<float>(textureHeight_);
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    commandList->RSSetViewports(1, &viewport);

    D3D12_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = textureWidth_;
    scissorRect.bottom = textureHeight_;
    commandList->RSSetScissorRects(1, &scissorRect);

    // RootSignature設定
    commandList->SetGraphicsRootSignature(copyImageRootSignature_.Get());

    // 処理モードに応じてPipelineState設定
    switch (processingMode_) {
    case ProcessingMode::Normal:
        commandList->SetPipelineState(copyImagePipelineState_.Get());
        break;
    case ProcessingMode::Grayscale:
        commandList->SetPipelineState(grayscalePipelineState_.Get());
        break;
    case ProcessingMode::Vignetting:
        commandList->SetPipelineState(vignettePipelineState_.Get());
        break;
    default:
        commandList->SetPipelineState(copyImagePipelineState_.Get());
        break;
    }

    // PrimitiveTopology設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 【重要修正】CommandKick後の新しいCommandListにDescriptor Heapをセット
    if (!srvManager_) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: srvManager_ is null!\n");
        return;
    }
    
    srvManager_->PreDraw();

    // テクスチャをシェーダーリソースとして設定
    renderTexture_->SetShaderResource(0);

    // DrawCall発行（資料より：VertexShaderは3頂点を想定しているので、DrawInstancedで3頂点描画する指定をする）
    commandList->DrawInstanced(3, 1, 0, 0);
    
    // 【重要修正】ImGui描画のため、SwapChainのリソース状態をRENDER_TARGETのままにしておく
    // 【重要修正】CommandKick()をここで呼ばない - ImGui描画のためにコマンドリストを保持
}