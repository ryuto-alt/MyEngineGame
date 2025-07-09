#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <array>

#include "base/ResourceDataStructure.h"

class DirectXCommon;
class RtvManager {
public:

	static const uint32_t kMaxRTVCount_; // RTVの最大数

public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* directXCommon);

	void Finalize();

	/// <summary>
	/// RTVインデックスの取得
	/// </summary>
	/// <returns></returns>
	uint32_t Allocate();

	/// <summary>
	/// 描画前処理
	/// </summary>
	//void SetDescriptorHeap();

	/// <summary>
	/// RTV生成
	/// </summary>
	void CreateRTV(ID3D12Resource* pResource, uint32_t rtvIndex);

	void CrearRenderTarget();
public:

	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvDescriptorHandleCPU(uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetRtvDescriptorHandleGPU(uint32_t index);

	ID3D12DescriptorHeap* GetRtvHeap() { return descriptorHeap_.Get(); }

	D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc() { return rtvDesc_; }

private:

	DirectXCommon* dxCommon_ = nullptr;

	//RTV用デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
	//デスクリプタ1個分のサイズ
	uint32_t descriptorSize_ = 0;
	//使用中のインデックス
	uint32_t useIndex_ = 0;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};
};

