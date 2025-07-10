#pragma once
//#include <d3d12.h>
//#include <dxgi1_6.h>
//#include <wrl.h>
#include "DirectXCommon.h"


class SrvManager
{
public:
/// <summary>
/// 初期化
/// </summary>
	void Initialize(DirectXCommon* dxcommon);

	//アロケータ（ヒープのアドレスを指定するやつ）
	uint32_t Allocate();

	//cpu、gpuの計算用関数
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	//SRV生成（テクスチャ用）
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT foemat, UINT MipLevels);
	//SRV生成(structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResourece, UINT numElements,
		UINT structureByteStride);
	void PreDraw();
	void SetGraficsRootDescriptorTable(UINT RootprameterIndex, uint32_t srvIndex);

	bool CheckTexturesNumber();
private:
	DirectXCommon* directXCommon = nullptr;
	//最大SRV数（最大テクスチャ枚数）
	static const uint32_t kMaxSRVCount;
	//SRV用のデスクリプタサイズ
	uint32_t descriptorSize;
	//SRV用のデスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	//次に使用するSRVインデックス
	uint32_t useIndex = 0;
	

};

