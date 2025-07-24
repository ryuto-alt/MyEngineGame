#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include "Vector4.h"

class DirectXCommon;
class SrvManager;

// レンダーテクスチャクラス
class RenderTexture {
public:
    // デストラクタ
    ~RenderTexture() = default;

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,
        uint32_t width, uint32_t height,
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        const Vector4& clearColor = { 1.0f, 0.0f, 0.0f, 1.0f });

    // レンダーターゲットとして設定開始
    void BeginRenderTarget();

    // レンダーターゲットとして設定終了
    void EndRenderTarget();

    // テクスチャとして使用するためのSRV設定
    void SetShaderResource(UINT rootParameterIndex);
    
    // リソース状態をSHADER_RESOURCEにする
    void EnsureShaderResourceState();

    // CPUハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle() const { return rtvHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandle() const { return srvHandleGPU; }

    // リソース取得
    Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return renderTextureResource; }

    // 幅・高さ取得
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    // フォーマット取得
    DXGI_FORMAT GetFormat() const { return format_; }

private:
    // RenderTextureResourceの作成
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(
        uint32_t width, uint32_t height, 
        DXGI_FORMAT format, const Vector4& clearColor);

    // 深度バッファリソースの作成
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilResource(
        uint32_t width, uint32_t height);

    // RTVの作成
    void CreateRTV();

    // DSVの作成
    void CreateDSV();

    // SRVの作成
    void CreateSRV();

private:
    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    // レンダーテクスチャリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;

    // 深度バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

    // RTV関連
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
    uint32_t rtvIndex = 0;

    // DSV関連
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{};
    uint32_t dsvIndex = 0;

    // SRV関連
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU{};
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU{};
    uint32_t srvIndex = 0;

    // テクスチャ情報
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    Vector4 clearColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };

    // 現在のResourceState
    D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
};