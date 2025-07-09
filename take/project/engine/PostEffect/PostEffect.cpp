#include "PostEffect.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void PostEffect::Initialize(
	DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) {

	dxCommon_ = dxCommon;
	//SRVManagerの取得
	srvManager_ = srvManager;

	csFilePath_ = CSFilePath;

	//PSO初期化
	computePSO_ = std::make_unique<PSO>();
	computePSO_->CompileComputeShader(dxCommon_->GetDXC(), csFilePath_);
	computePSO_->CreateComputePSO(dxCommon_->GetDevice());
	rootSignature_ = computePSO_->GetComputeRootSignature();

	//inputRenderTexture
	inputTexSrvIndex_ = inputSrvIdx;
	inputResource_ = inputResource;
	//srv生成
	srvManager_->CreateSRVforRenderTexture(inputResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM, inputTexSrvIndex_);
	//outputRenderTexture
	outputTexSrvIndex_ = srvManager_->Allocate();
	outputTexUavIndex_ = srvManager_->Allocate();
	outputResource_ = outputResource;
	//srv生成
	srvManager_->CreateSRVforRenderTexture(outputResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, outputTexSrvIndex_);
	//uav生成
	srvManager_->CreateUAVforRenderTexture(outputResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM, outputTexUavIndex_);
}

void PostEffect::UpdateImGui() {
	
}

void PostEffect::DisPatch() {
}