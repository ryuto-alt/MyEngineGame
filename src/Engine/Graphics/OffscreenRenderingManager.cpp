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
    if (sepiaPixelShader_) {
        sepiaPixelShader_->Release();
        sepiaPixelShader_ = nullptr;
    }
}

void OffscreenRenderingManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,
    uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
    
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Starting initialization\n");
    
    assert(dxCommon);
    assert(srvManager);

    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
    textureWidth_ = width;
    textureHeight_ = height;

    char initDebug[256];
    sprintf_s(initDebug, "OffscreenRenderingManager::Initialize - Size: %dx%d, Format: %d\n", width, height, format);
    OutputDebugStringA(initDebug);

    // RenderTextureの作成
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Creating RenderTexture\n");
    renderTexture_ = std::make_unique<RenderTexture>();
    
    if (!renderTexture_) {
        OutputDebugStringA("OffscreenRenderingManager::Initialize - ERROR: Failed to create RenderTexture!\n");
        return;
    }
    
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Initializing RenderTexture\n");
    renderTexture_->Initialize(dxCommon_, srvManager_, width, height, format, clearColor);
    OutputDebugStringA("OffscreenRenderingManager::Initialize - RenderTexture initialized\n");

    // シェーダーコンパイル
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Compiling shaders\n");
    CompileShaders();
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Shaders compiled\n");

    // RootSignatureの作成
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Creating root signature\n");
    CreateRootSignature();
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Root signature created\n");

    // 各種パイプラインステートオブジェクトを作成
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Creating pipeline states\n");
    CreateCopyImagePSO();
    CreateGrayscalePSO();
    CreateSepiaPSO();
    OutputDebugStringA("OffscreenRenderingManager::Initialize - All pipeline states created\n");
    
    OutputDebugStringA("OffscreenRenderingManager::Initialize - Initialization completed successfully\n");
}

void OffscreenRenderingManager::CompileShaders() {
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Starting shader compilation\n");
    
    // フルスクリーン処理用の共通VertexShaderをコンパイル
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Compiling Fullscreen.VS.hlsl\n");
    copyImageVertexShader_ = dxCommon_->CompileShader(L"Resources/shaders/Fullscreen.VS.hlsl", L"vs_6_0");
    if (!copyImageVertexShader_) {
        OutputDebugStringA("ERROR: Failed to compile Fullscreen.VS.hlsl\n");
        assert(false);
    }
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Fullscreen.VS.hlsl compiled successfully\n");
    
    // CopyImage用PixelShaderをコンパイル
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Compiling CopyImage.PS.hlsl\n");
    copyImagePixelShader_ = dxCommon_->CompileShader(L"Resources/shaders/CopyImage.PS.hlsl", L"ps_6_0");
    if (!copyImagePixelShader_) {
        OutputDebugStringA("ERROR: Failed to compile CopyImage.PS.hlsl\n");
        assert(false);
    }
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - CopyImage.PS.hlsl compiled successfully\n");
    
    // 【テスト用】BlueTest用PixelShaderをコンパイル（確実な動作確認のため）
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Compiling BlueTest.PS.hlsl for testing\n");
    grayscalePixelShader_ = dxCommon_->CompileShader(L"Resources/shaders/BlueTest.PS.hlsl", L"ps_6_0");
    if (!grayscalePixelShader_) {
        OutputDebugStringA("ERROR: Failed to compile BlueTest.PS.hlsl\n");
        // Grayscale.PS.hlslにフォールバック
        OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Fallback to Grayscale.PS.hlsl\n");
        grayscalePixelShader_ = dxCommon_->CompileShader(L"Resources/shaders/Grayscale.PS.hlsl", L"ps_6_0");
        if (!grayscalePixelShader_) {
            OutputDebugStringA("ERROR: Failed to compile Grayscale.PS.hlsl as well\n");
            assert(false);
        }
    }
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Test shader compiled successfully\n");
    
    // Sepia用PixelShaderをコンパイル
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Compiling Sepia.PS.hlsl\n");
    sepiaPixelShader_ = dxCommon_->CompileShader(L"Resources/shaders/Sepia.PS.hlsl", L"ps_6_0");
    if (!sepiaPixelShader_) {
        OutputDebugStringA("ERROR: Failed to compile Sepia.PS.hlsl\n");
        assert(false);
    }
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - Sepia.PS.hlsl compiled successfully\n");
    
    OutputDebugStringA("OffscreenRenderingManager::CompileShaders - All shaders compiled successfully\n");
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

void OffscreenRenderingManager::CreateSepiaPSO() {
    // セピア調用のパイプラインステート作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    // RootSignature（共通のものを使用）
    pipelineStateDesc.pRootSignature = copyImageRootSignature_.Get();

    // InputLayout（共通：頂点データなし）
    pipelineStateDesc.InputLayout.pInputElementDescs = nullptr;
    pipelineStateDesc.InputLayout.NumElements = 0;

    // Shader（VertexShaderは共通、PixelShaderはセピア用）
    pipelineStateDesc.VS.pShaderBytecode = copyImageVertexShader_->GetBufferPointer();
    pipelineStateDesc.VS.BytecodeLength = copyImageVertexShader_->GetBufferSize();
    pipelineStateDesc.PS.pShaderBytecode = sepiaPixelShader_->GetBufferPointer();
    pipelineStateDesc.PS.BytecodeLength = sepiaPixelShader_->GetBufferSize();

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
        IID_PPV_ARGS(&sepiaPipelineState_));
    assert(SUCCEEDED(hr));
}

void OffscreenRenderingManager::BeginRenderToTexture() {
    // フレーム開始時の状態管理強化
    OutputDebugStringA("OffscreenRenderingManager::BeginRenderToTexture - Frame start\n");
    
    // リソースの有効性チェック
    if (!renderTexture_) {
        OutputDebugStringA("OffscreenRenderingManager::BeginRenderToTexture - ERROR: renderTexture_ is null!\n");
        return;
    }
    
    if (!dxCommon_) {
        OutputDebugStringA("OffscreenRenderingManager::BeginRenderToTexture - ERROR: dxCommon_ is null!\n");
        return;
    }
    
    OutputDebugStringA("OffscreenRenderingManager::BeginRenderToTexture - About to call renderTexture_->BeginRenderTarget()\n");
    
    // RenderTextureをレンダーターゲットとして設定
    renderTexture_->BeginRenderTarget();
    
    OutputDebugStringA("OffscreenRenderingManager::BeginRenderToTexture - renderTexture_->BeginRenderTarget() completed\n");
}

void OffscreenRenderingManager::EndRenderToTexture() {
    OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - Starting\n");
    
    if (!renderTexture_) {
        OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - ERROR: renderTexture_ is null!\n");
        return;
    }
    
    if (!dxCommon_) {
        OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - ERROR: dxCommon_ is null!\n");
        return;
    }
    
    OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - About to call renderTexture_->EndRenderTarget()\n");
    
    // RenderTextureのレンダーターゲット使用を終了
    renderTexture_->EndRenderTarget();
    
    OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - renderTexture_->EndRenderTarget() completed\n");
    
    // 【重要】D3D12エラー修正：オフスクリーンレンダリング完了後にコマンドリストを実行
    // 同一コマンドリストで複数のSwapChainバッファへの書き込みを避けるため
    OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - About to call dxCommon_->CommandKick()\n");
    dxCommon_->CommandKick();
    OutputDebugStringA("OffscreenRenderingManager::EndRenderToTexture - dxCommon_->CommandKick() completed\n");
}

void OffscreenRenderingManager::CopyToSwapChain() {
    OutputDebugStringA("*** OffscreenRenderingManager::CopyToSwapChain - STARTING COPY OPERATION ***\n");
    
    // D3D12エラー修正：EndRenderToTexture()でCommandKick()を呼び出したため、
    // ここでは新しいコマンドリストを使用してSwapChainへのコピーを実行
    if (!dxCommon_) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: dxCommon_ is null!\n");
        return;
    }
    
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
    if (!commandList) {
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - ERROR: commandList is null!\n");
        return;
    }
    
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Got command list\n");
    
    // 【重要修正】SwapChainのリソース状態をPRESENTからRENDER_TARGETに遷移
    UINT backBufferIndex = dxCommon_->GetSwapChain()->GetCurrentBackBufferIndex();
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource = dxCommon_->GetSwapChainResource(backBufferIndex);
    
    // リソースバリアを作成してPRESENTからRENDER_TARGETに遷移
    D3D12_RESOURCE_BARRIER presentToRenderTarget{};
    presentToRenderTarget.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    presentToRenderTarget.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    presentToRenderTarget.Transition.pResource = swapChainResource.Get();
    presentToRenderTarget.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    presentToRenderTarget.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &presentToRenderTarget);
    
    // 【重要】SwapChainを明示的にRenderTargetとして設定（RenderTextureのアクティブ状態を解除）
    D3D12_CPU_DESCRIPTOR_HANDLE swapChainRtvHandle = dxCommon_->GetRTVCPUDescriptorHandle(backBufferIndex);
    commandList->OMSetRenderTargets(1, &swapChainRtvHandle, FALSE, nullptr);
    
    // 【重要修正】スワップチェインを黒色でクリアして確実に描画される状態にする
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 黒色
    commandList->ClearRenderTargetView(swapChainRtvHandle, clearColor, 0, nullptr);
    
    // デバッグ出力
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - SwapChain set as RenderTarget and cleared\n");

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
    char modeDebug[256];
    sprintf_s(modeDebug, "OffscreenRenderingManager::CopyToSwapChain - Current processing mode: %d\n", (int)processingMode_);
    OutputDebugStringA(modeDebug);
    
    switch (processingMode_) {
    case ProcessingMode::Normal:
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Setting Normal pipeline\n");
        if (!copyImagePipelineState_) {
            OutputDebugStringA("ERROR: copyImagePipelineState_ is null!\n");
        }
        commandList->SetPipelineState(copyImagePipelineState_.Get());
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Normal pipeline set successfully\n");
        break;
    case ProcessingMode::Grayscale:
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Setting Grayscale pipeline\n");
        if (!grayscalePipelineState_) {
            OutputDebugStringA("ERROR: grayscalePipelineState_ is null!\n");
        }
        commandList->SetPipelineState(grayscalePipelineState_.Get());
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Grayscale pipeline set successfully\n");
        break;
    case ProcessingMode::Sepia:
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Setting Sepia pipeline\n");
        if (!sepiaPipelineState_) {
            OutputDebugStringA("ERROR: sepiaPipelineState_ is null!\n");
        }
        commandList->SetPipelineState(sepiaPipelineState_.Get());
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Sepia pipeline set successfully\n");
        break;
    default:
        OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Using default Normal mode (unknown mode)\n");
        commandList->SetPipelineState(copyImagePipelineState_.Get());
        break;
    }

    // PrimitiveTopology設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 【重要修正】CommandKick後の新しいCommandListにDescriptor Heapをセット
    // これがないとSetGraphicsRootDescriptorTableでエラーが発生する
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - About to call srvManager_->PreDraw()\n");
    srvManager_->PreDraw();
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - srvManager_->PreDraw() completed\n");

    // テクスチャをシェーダーリソースとして設定
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - About to call renderTexture_->SetShaderResource()\n");
    renderTexture_->SetShaderResource(0);
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - renderTexture_->SetShaderResource() completed\n");

    // DrawCall発行（資料より：VertexShaderは3頂点を想定しているので、DrawInstancedで3頂点描画する指定をする）
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - About to call DrawInstanced\n");
    commandList->DrawInstanced(3, 1, 0, 0);
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - DrawInstanced completed\n");
    
    // 【重要修正】SwapChainのリソース状態をRENDER_TARGETからPRESENTに戻す
    D3D12_RESOURCE_BARRIER renderTargetToPresent{};
    renderTargetToPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    renderTargetToPresent.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    renderTargetToPresent.Transition.pResource = swapChainResource.Get();
    renderTargetToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    renderTargetToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList->ResourceBarrier(1, &renderTargetToPresent);
    
    // 【重要修正】SwapChain描画完了後にコマンドリストを実行・リセット
    // 同一コマンドリストでの複数SwapChainバッファアクセスを避けるため
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - About to call final CommandKick()\n");
    dxCommon_->CommandKick();
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - Final CommandKick() completed\n");
    
    OutputDebugStringA("OffscreenRenderingManager::CopyToSwapChain - All operations completed successfully\n");
}