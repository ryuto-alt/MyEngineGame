#include "RenderTexture.h"
#include "DirectXCommon.h"
#include "SRVManager.h"
#include "d3dx12.h"
#include <cassert>

void RenderTexture::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,
    uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
    
    OutputDebugStringA("RenderTexture::Initialize - Starting initialization\n");
    
    assert(dxCommon);
    assert(srvManager);

    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
    width_ = width;
    height_ = height;
    format_ = format;
    clearColor_ = clearColor;

    char rtInitDebug[256];
    sprintf_s(rtInitDebug, "RenderTexture::Initialize - Size: %dx%d, Format: %d\n", width, height, format);
    OutputDebugStringA(rtInitDebug);

    // RenderTextureResourceの作成
    OutputDebugStringA("RenderTexture::Initialize - Creating render texture resource\n");
    renderTextureResource = CreateRenderTextureResource(width, height, format, clearColor);
    
    if (!renderTextureResource) {
        OutputDebugStringA("RenderTexture::Initialize - ERROR: Failed to create render texture resource!\n");
        return;
    }
    
    OutputDebugStringA("RenderTexture::Initialize - Render texture resource created\n");

    // 初期状態を明示的に設定（リソース作成時の状態と一致させる）
    currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    OutputDebugStringA("RenderTexture::Initialize - Initial state set to RENDER_TARGET\n");

    // RTVの作成
    OutputDebugStringA("RenderTexture::Initialize - Creating RTV\n");
    CreateRTV();
    OutputDebugStringA("RenderTexture::Initialize - RTV created\n");

    // SRVの作成
    OutputDebugStringA("RenderTexture::Initialize - Creating SRV\n");
    CreateSRV();
    OutputDebugStringA("RenderTexture::Initialize - SRV created\n");

    // 深度バッファリソースの作成
    OutputDebugStringA("RenderTexture::Initialize - Creating depth stencil resource\n");
    depthStencilResource = CreateDepthStencilResource(width, height);
    OutputDebugStringA("RenderTexture::Initialize - Depth stencil resource created\n");

    // DSVの作成
    OutputDebugStringA("RenderTexture::Initialize - Creating DSV\n");
    CreateDSV();
    OutputDebugStringA("RenderTexture::Initialize - DSV created\n");
    
    OutputDebugStringA("RenderTexture::Initialize - Initialization completed successfully\n");
}

Microsoft::WRL::ComPtr<ID3D12Resource> RenderTexture::CreateRenderTextureResource(
    uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
    
    // 資料に基づく実装：なにはともあれRenderTexture生成の関数を作る
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.MipLevels = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format = format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして利用可能にする

    // HeapはDEFAULTで作る
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    // ClearValueの設定（資料より）
    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = format;
    clearValue.Color[0] = clearColor.x;
    clearValue.Color[1] = clearColor.y;
    clearValue.Color[2] = clearColor.z;
    clearValue.Color[3] = clearColor.w;

    // Resourceの作成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, // RenderTargetとして使うので、初期StateはD3D12_RESOURCE_STATE_RENDER_TARGET
        &clearValue,
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));

    return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> RenderTexture::CreateDepthStencilResource(
    uint32_t width, uint32_t height) {
    
    // 深度バッファリソースの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.MipLevels = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 3Dオブジェクトが期待するフォーマット
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // ヒーププロパティ
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    // クリア値の設定
    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    // リソースの作成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度バッファの初期状態
        &clearValue,
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));

    char debugMsg[256];
    sprintf_s(debugMsg, "RenderTexture::CreateDepthStencilResource - Created depth buffer: %dx%d\n", width, height);
    OutputDebugStringA(debugMsg);

    return resource;
}

void RenderTexture::CreateRTV() {
    // RTVの作成（新しいRTV管理システムを使用）
    // RTVインデックスを動的に確保
    rtvIndex = dxCommon_->AllocateRTVIndex();
    
    // RTVハンドルを取得
    rtvHandle = dxCommon_->GetRTVCPUDescriptorHandle(rtvIndex);

    // DirectXCommonの新しいRTV作成機能を使用
    dxCommon_->CreateRenderTargetViewAt(rtvIndex, renderTextureResource, format_);
}

void RenderTexture::CreateSRV() {
    // SRVの作成（資料より）
    // RenderTextureに書き込んだ内容を読むためにSRVが必要
    
    // SRVインデックスの確保
    srvIndex = srvManager_->Allocate();

    // SRVハンドルの取得
    srvHandleCPU = srvManager_->GetCPUDescriptorHandle(srvIndex);
    srvHandleGPU = srvManager_->GetGPUDescriptorHandle(srvIndex);

    // SRVの設定。FormatはResourceと同じにしておく
    srvManager_->CreateSRVForTexture2D(
        srvIndex,
        renderTextureResource,
        format_,
        1 // MipLevels
    );
}

void RenderTexture::CreateDSV() {
    // DSVの作成
    // インデックス1を使用（0はDirectXCommonのメイン深度バッファ用）
    dsvIndex = 1;
    
    // DSVハンドルを取得
    dsvHandle = dxCommon_->GetDSVCPUDescriptorHandle(dsvIndex);

    // DSVビューの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    // DSVを作成
    dxCommon_->GetDevice()->CreateDepthStencilView(
        depthStencilResource.Get(),
        &dsvDesc,
        dsvHandle
    );

    char debugMsg[256];
    sprintf_s(debugMsg, "RenderTexture::CreateDSV - DSVIndex: %d, Handle ptr: %llu\n", dsvIndex, dsvHandle.ptr);
    OutputDebugStringA(debugMsg);
}

void RenderTexture::BeginRenderTarget() {
    // ResourceBarrierを張る（資料より）
    // コピーのために読まれる（PixelShaderResource）から Sceneが書き込まれ（RenderTarget）へ
    
    OutputDebugStringA("RenderTexture::BeginRenderTarget - Starting...\n");
    
    // リソースの有効性チェック
    if (!renderTextureResource) {
        OutputDebugStringA("RenderTexture::BeginRenderTarget - ERROR: renderTextureResource is null!\n");
        return;
    }
    
    if (!dxCommon_) {
        OutputDebugStringA("RenderTexture::BeginRenderTarget - ERROR: dxCommon_ is null!\n");
        return;
    }
    
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
    if (!commandList) {
        OutputDebugStringA("RenderTexture::BeginRenderTarget - ERROR: commandList is null!\n");
        return;
    }
    
    OutputDebugStringA("RenderTexture::BeginRenderTarget - Resource validation passed\n");
    
    // 【重要】フレーム間での状態管理を強化
    // 前フレームでPIXEL_SHADER_RESOURCEの状態から確実にRENDER_TARGETに遷移
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = renderTextureResource.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = currentState;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // デバッグ出力で状態遷移を追跡
    char debugMsg[256];
    sprintf_s(debugMsg, "RenderTexture::BeginRenderTarget - StateBefore: %d, StateAfter: %d\n", 
        currentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
    OutputDebugStringA(debugMsg);

    // 常にResourceBarrierを実行（フレーム間での状態不整合を防ぐため）
    if (currentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
        OutputDebugStringA("RenderTexture::BeginRenderTarget - About to execute ResourceBarrier\n");
        commandList->ResourceBarrier(1, &barrier);
        OutputDebugStringA("RenderTexture: ResourceBarrier executed (PIXEL_SHADER_RESOURCE -> RENDER_TARGET)\n");
    } else {
        OutputDebugStringA("RenderTexture: ResourceBarrier skipped (already RENDER_TARGET)\n");
    }
    
    // 状態を確実に更新
    currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    OutputDebugStringA("RenderTexture::BeginRenderTarget - State updated to RENDER_TARGET\n");

    // レンダーターゲットとして設定
    OutputDebugStringA("RenderTexture::BeginRenderTarget - About to call OMSetRenderTargets\n");
    
    // RTVハンドルの有効性チェック
    if (rtvHandle.ptr == 0) {
        OutputDebugStringA("RenderTexture::BeginRenderTarget - ERROR: rtvHandle.ptr is 0 (invalid handle)!\n");
        return;
    }
    
    char handleDebug[128];
    sprintf_s(handleDebug, "RenderTexture::BeginRenderTarget - RTVHandle ptr: %llu\n", rtvHandle.ptr);
    OutputDebugStringA(handleDebug);
    
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    OutputDebugStringA("RenderTexture: OMSetRenderTargets executed with depth stencil view\n");

    // 【重要修正】オフスクリーンレンダリング用のビューポートとシザー矩形を設定
    D3D12_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_); 
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    commandList->RSSetViewports(1, &viewport);

    D3D12_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = width_;
    scissorRect.bottom = height_;
    commandList->RSSetScissorRects(1, &scissorRect);
    
    OutputDebugStringA("RenderTexture: Viewport and scissor rect set for offscreen rendering\n");

    // レンダーターゲットをクリア
    OutputDebugStringA("RenderTexture::BeginRenderTarget - About to call ClearRenderTargetView\n");
    commandList->ClearRenderTargetView(
        rtvHandle,
        &clearColor_.x,
        0,
        nullptr
    );
    OutputDebugStringA("RenderTexture::BeginRenderTarget - ClearRenderTargetView executed\n");

    // 深度バッファをクリア
    OutputDebugStringA("RenderTexture::BeginRenderTarget - About to call ClearDepthStencilView\n");
    commandList->ClearDepthStencilView(
        dsvHandle,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f, // 深度クリア値（最大値）
        0,    // ステンシルクリア値
        0,
        nullptr
    );
    OutputDebugStringA("RenderTexture::BeginRenderTarget - ClearDepthStencilView executed\n");
    
    OutputDebugStringA("RenderTexture::BeginRenderTarget completed\n");
}

void RenderTexture::EndRenderTarget() {
    // ResourceBarrierを張る（資料より）
    // Sceneが書き込まれ（RenderTarget）から コピーのために読まれる（PixelShaderResource）へ
    
    // 【重要】RenderTextureをアクティブなRenderTargetから確実に外す
    // nullptrをRenderTargetとして設定してRenderTextureの参照を解除
    dxCommon_->GetCommandList()->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
    OutputDebugStringA("RenderTexture: OMSetRenderTargets reset to nullptr\n");
    
    // デバッグ出力で状態遷移を追跡
    char debugMsg[256];
    sprintf_s(debugMsg, "RenderTexture::EndRenderTarget - StateBefore: %d, StateAfter: %d\n", 
        currentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    OutputDebugStringA(debugMsg);
    
    if (currentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTextureResource.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = currentState;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
        OutputDebugStringA("RenderTexture: ResourceBarrier executed (RENDER_TARGET -> PIXEL_SHADER_RESOURCE)\n");
        currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    } else {
        OutputDebugStringA("RenderTexture: ResourceBarrier skipped (already PIXEL_SHADER_RESOURCE)\n");
    }
    
    OutputDebugStringA("RenderTexture::EndRenderTarget completed\n");
}

void RenderTexture::SetShaderResource(UINT rootParameterIndex) {
    // デバッグ出力で現在の状態を確認
    char debugMsg[256];
    sprintf_s(debugMsg, "RenderTexture::SetShaderResource - CurrentState: %d, RootParam: %d\n", 
        currentState, rootParameterIndex);
    OutputDebugStringA(debugMsg);
    
    // SRVとして設定
    srvManager_->SetGraphicsRootDescriptorTable(rootParameterIndex, srvIndex);
    
    OutputDebugStringA("RenderTexture::SetShaderResource completed\n");
}