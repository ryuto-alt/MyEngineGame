#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{

public:
	// コンストラクタ
	SpriteCommon() = default;
	// デストラクタ
	~SpriteCommon();

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	// 共通描画設定
	void CommonDraw();

	DirectXCommon* GetDxCommon()const { return dxCommon_; }


	// ルートシグネチャを取得
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const { return rootSignature; }
	
	// スキニング用パイプラインを取得
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetSkinningPipelineState() const { return skinningPipelineState; }

private:
	// ルートシグネチャの作成
	void RootSignatureInitialize();

	// グラフィックスパイプライン
	void GraphicsPipelineInitialize();
	
	// スキニング用パイプライン
	void SkinningPipelineInitialize();

private:
	DirectXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningPipelineState = nullptr;
};