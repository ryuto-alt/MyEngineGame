#include "Vignette.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void Vignette::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx,outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("VignettePSO");
	//Bufferの名前付け
	inputResource_->SetName(L"Vignette::inputResource_");
	outputResource_->SetName(L"Vignette::outputResource_");

	//VignetteScaleResource
	vignetteInfoResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(VignetteInfo));
	vignetteInfoResource_->SetName(L"Vignette::vignetteInfoResource_");
	//Mapping
	vignetteInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&vignetteInfoData_));
}

void Vignette::UpdateImGui() {
#ifdef _DEBUG

	ImGui::Begin("Vignette");
	ImGui::Text("VignetteScale");
	ImGui::SliderFloat("VignetteScale", &vignetteInfoData_->vignetteScale, 0.0f, 3.0f,"%.5f");
	ImGui::Text("VignettePower");
	ImGui::SliderFloat("VignettePower", &vignetteInfoData_->vignettePower, 0.1f, 1.0f);
	ImGui::Text("VignetteFlag");
	ImGui::Checkbox("VignetteFlag", &vignetteInfoData_->flag);
	ImGui::End();
#endif // _DEBUG

}

void Vignette::DisPatch() {

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
	//VignetteInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(computePSO_->GetComputeBindResourceIndex("gVignetteInfo"), vignetteInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());

}
