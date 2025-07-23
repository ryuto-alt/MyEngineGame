#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <dxcapi.h>
#include "RenderTexture.h"
#include "Vector4.h"

class DirectXCommon;
class SrvManager;

// 処理モード
enum class ProcessingMode {
    Normal,      // 通常（そのままコピー）
    Grayscale,   // グレースケール
    Sepia        // セピア調
};

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

    // 処理モードの設定
    void SetProcessingMode(ProcessingMode mode) { processingMode_ = mode; }
    
    // 現在の処理モード取得
    ProcessingMode GetProcessingMode() const { return processingMode_; }

private:
    // 各種パイプラインステートオブジェクトを作成
    void CreateCopyImagePSO();
    void CreateGrayscalePSO();
    void CreateSepiaPSO();

    // RootSignatureの作成
    void CreateRootSignature();

    // シェーダーコンパイル
    void CompileShaders();

private:
    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    // RenderTexture
    std::unique_ptr<RenderTexture> renderTexture_;

    // 共通のRootSignature（全ての処理で同じものを使用）
    Microsoft::WRL::ComPtr<ID3D12RootSignature> copyImageRootSignature_;
    
    // 各種パイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> copyImagePipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> grayscalePipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> sepiaPipelineState_;

    // シェーダー
    IDxcBlob* copyImageVertexShader_ = nullptr;  // 共通のVertex Shader
    IDxcBlob* copyImagePixelShader_ = nullptr;
    IDxcBlob* grayscalePixelShader_ = nullptr;
    IDxcBlob* sepiaPixelShader_ = nullptr;
    
    // 現在の処理モード
    ProcessingMode processingMode_ = ProcessingMode::Normal;

    // テクスチャサイズ
    uint32_t textureWidth_ = 0;
    uint32_t textureHeight_ = 0;
};