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
    Vignetting,  // ヴィネッティング（周囲を暗くする効果）
    Horror       // ホラー効果（VHSノイズ、色収差、血しぶき）
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
    void CreateVignettePSO();
    void CreateHorrorPSO();

    // RootSignatureの作成
    void CreateRootSignature();
    void CreateHorrorRootSignature();

    // シェーダーコンパイル
    void CompileShaders();
    
    // ホラー効果用Constant Bufferの作成
    void CreateHorrorConstantBuffer();
    
    // ホラー効果用Constant Bufferの更新
    void UpdateHorrorConstantBuffer();

private:
    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;

    // RenderTexture
    std::unique_ptr<RenderTexture> renderTexture_;

    // 共通のRootSignature（Normal, Grayscale, Vignetting用）
    Microsoft::WRL::ComPtr<ID3D12RootSignature> copyImageRootSignature_;
    
    // Horror用のRootSignature（Constant Bufferを含む）
    Microsoft::WRL::ComPtr<ID3D12RootSignature> horrorRootSignature_;
    
    // 各種パイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> copyImagePipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> grayscalePipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> vignettePipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> horrorPipelineState_;

    // シェーダー
    IDxcBlob* copyImageVertexShader_ = nullptr;  // 共通のVertex Shader
    IDxcBlob* copyImagePixelShader_ = nullptr;
    IDxcBlob* grayscalePixelShader_ = nullptr;
    IDxcBlob* vignettePixelShader_ = nullptr;
    IDxcBlob* horrorPixelShader_ = nullptr;
    
    // 現在の処理モード
    ProcessingMode processingMode_ = ProcessingMode::Normal;

    // テクスチャサイズ
    uint32_t textureWidth_ = 0;
    uint32_t textureHeight_ = 0;
    
    // ホラーエフェクトパラメータ
    struct HorrorParams {
        float time = 0.0f;
        float noiseIntensity = 0.5f;
        float distortionAmount = 0.3f;
        float bloodAmount = 0.4f;
    };
    HorrorParams horrorParams_;
    Microsoft::WRL::ComPtr<ID3D12Resource> horrorConstantBuffer_;
    
    // 経過時間（ホラーエフェクト用）
    float elapsedTime_ = 0.0f;
    
public:
    // ホラーエフェクトパラメータの設定
    void SetHorrorParams(float noiseIntensity, float distortionAmount, float bloodAmount) {
        horrorParams_.noiseIntensity = noiseIntensity;
        horrorParams_.distortionAmount = distortionAmount;
        horrorParams_.bloodAmount = bloodAmount;
    }
    
    // デルタタイムの更新（アニメーション用）
    void UpdateTime(float deltaTime) {
        elapsedTime_ += deltaTime;
        horrorParams_.time = elapsedTime_;
    }
};