#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <dxcapi.h>
#include "RenderTexture.h"
#include "Vector4.h"

class DirectXCommon;
class SrvManager;

// オフスクリーンレンダリング管理クラス
class OffscreenRenderingManager {
public:
    // デストラクタ
    ~OffscreenRenderingManager();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,
        uint32_t width, uint32_t height,
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        const Vector4& clearColor = { 1.0f, 0.0f, 0.0f, 1.0f });

    // レンダーテクスチャへの描画開始
    void BeginRenderToTexture();

    // レンダーテクスチャへの描画終了
    void EndRenderToTexture();

    // SwapChainへのコピー実行
    void CopyToSwapChain();

    // RenderTextureの取得
    RenderTexture* GetRenderTexture() const { return renderTexture_.get(); }

private:
    // CopyImage用のパイプラインステートオブジェクトを作成
    void CreateCopyImagePSO();

    // RootSignatureの作成
    void CreateRootSignature();

    // シェーダーコンパイル
    void CompileShaders();

private:
    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    // RenderTexture
    std::unique_ptr<RenderTexture> renderTexture_;

    // CopyImage用のパイプライン
    Microsoft::WRL::ComPtr<ID3D12RootSignature> copyImageRootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> copyImagePipelineState_;

    // シェーダー
    IDxcBlob* copyImageVertexShader_ = nullptr;
    IDxcBlob* copyImagePixelShader_ = nullptr;

    // テクスチャサイズ
    uint32_t textureWidth_ = 0;
    uint32_t textureHeight_ = 0;
};