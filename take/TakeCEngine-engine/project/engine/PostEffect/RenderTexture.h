#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/PipelineStateObject.h"
#include "PostEffect/PostEffectManager.h"

class RenderTexture {
public:

	RenderTexture() = default;
	~RenderTexture();
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, PostEffectManager* postEffectManager);

	void ClearRenderTarget();

	void PreDraw();

	void Draw();

	void PostDraw();

	ComPtr<ID3D12Resource> GetRenderTextureResource() const {
		return renderTextureResource_;
	}

	uint32_t GetSrvIndex() const {return srvIndex_;}

private:

	DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ
	RtvManager* rtvManager_ = nullptr; //RtvManagerのポインタ
	PostEffectManager* postEffectManager_ = nullptr; //PostEffectManagerのポインタ
	//clearValue
	//clearValue
	const Vector4 kRenderTargetClearColor_ = { 0.1f, 0.25f, 0.5f, 1.0f };
	const float clearValue_[4] = { kRenderTargetClearColor_.x, kRenderTargetClearColor_.y, kRenderTargetClearColor_.z, kRenderTargetClearColor_.w };

	//RTVのハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
	//DSVのハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
	//RTVのインデックス
	uint32_t rtvIndex_ = {};
	//SRVのインデックス
	uint32_t srvIndex_ = 0;

	ComPtr<ID3D12Resource> renderTextureResource_; 
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_;

	std::unique_ptr<PSO> renderTexturePSO_; //PSO
	ComPtr<ID3D12RootSignature> rootSignature_; //ルートシグネチャ
};