#include "rtvManager.h"
#include "DirectXCommon.h"
#include <cassert>
const uint32_t RtvManager::kMaxRTVCount_ = 16;

//===================================================================================================
// 初期化
//===================================================================================================

void RtvManager::Initialize(DirectXCommon* directXCommon) {
	//dxCommon_の取得
	dxCommon_ = directXCommon;
	//ディスクリプタヒープの生成
	descriptorHeap_= dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kMaxRTVCount_, false);
	//デスクリプタ1個分のサイズを取得して記録
	descriptorSize_ = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
}

void RtvManager::Finalize() {
	//ディスクリプタヒープの開放
	descriptorHeap_.Reset();
	//DirectXCommonのポインタをnullptrに
	dxCommon_ = nullptr;
	//使用中のインデックスを0に戻す
	useIndex_ = 0;
}

//===================================================================================================
// RTVインデックスの取得
//===================================================================================================

uint32_t RtvManager::Allocate() {
	
	//上限に達してないかチェック
	assert(useIndex_ < kMaxRTVCount_);

	//returnする番号をいったん記録
	int index = useIndex_;
	//次回のために番号を1進める
	useIndex_++;
	//記録した番号をreturn
	return index;
}

//==================================================================================================
// RTV生成
//==================================================================================================

void RtvManager::CreateRTV(ID3D12Resource* pResource, uint32_t rtvIndex) {	//RTVの生成

	
	
	//ディスクリプタハンドルの取得
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = GetRtvDescriptorHandleCPU(rtvIndex);
	//RTVの生成
	dxCommon_->GetDevice()->CreateRenderTargetView(pResource, &rtvDesc_, handleCPU);
}

void RtvManager::CrearRenderTarget() {

}

//===================================================================================================
// CPUディスクリプタハンドルの取得
//===================================================================================================

D3D12_CPU_DESCRIPTOR_HANDLE RtvManager::GetRtvDescriptorHandleCPU(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

//===================================================================================================
// GPUディスクリプタハンドルの取得
//===================================================================================================

D3D12_GPU_DESCRIPTOR_HANDLE RtvManager::GetRtvDescriptorHandleGPU(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}