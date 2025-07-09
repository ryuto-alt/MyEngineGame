#include "GrayScale.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void GrayScale::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx,outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("GrayScalePSO");
	//Bufferの名前付け
	inputResource_->SetName(L"GrayScale::inputResource_");
	outputResource_->SetName(L"GrayScale::outputResource_");
	//grayScaleTypeResource
	grayScaleTypeResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(int32_t));
	grayScaleTypeResource_->SetName(L"grayScaleTypeResource_");
	//Mapping
	grayScaleTypeResource_->Map(0, nullptr, reinterpret_cast<void**>(&grayScaleTypeData_));
	*grayScaleTypeData_ = 2;
}

void GrayScale::UpdateImGui() {
	#ifdef _DEBUG

	ImGui::Begin("GrayScale");
	ImGui::Text("GrayScaleType");
	ImGui::SliderInt("GrayScaleType", grayScaleTypeData_, 0, 2);
	ImGui::End();
#endif
}

void GrayScale::DisPatch() {


	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		outputResource_.Get());

	//Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(computePSO_->GetComputePipelineState());
	//inputTex
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gInputTexture"), inputTexSrvIndex_);
	//outputTex
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gOutputTexture"), outputTexUavIndex_);
	//grayScaleType
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(computePSO_->GetComputeBindResourceIndex("gType"), grayScaleTypeResource_->GetGPUVirtualAddress());

	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
