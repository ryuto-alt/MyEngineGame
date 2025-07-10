#pragma once
#include"DirectXCommon.h"
#include"SrvManager.h"
#include "GraphicsPipeline.h"

class OfscreenRenderManager
{
public:
	//初期化
	void Initialize(DirectXCommon* dxcommon, SrvManager*srvmanager);
	//描画前処理
	void Begin();
	//描画後処理
	void End();
	
	void Draw();
	
	
	//RenderTargetTextureの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTargetTextureResource(uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& ClearColor);


private:

	//DirectXCommonのポインタ
	DirectXCommon* dxCommon_ = nullptr;
	//SRVManagerのポインタ
	SrvManager* srvManager_ = nullptr;
	//レンダーテクスチャ
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTextureResource;//レンダーテクスチャ
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetTextureHandle;//レンダーテクスチャのハンドル
	const Vector4 clearColor = { 0.1f,0.25f,0.5f,1.0f };//とりあえず赤
	uint32_t srvIndex = 0;

	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;


};

