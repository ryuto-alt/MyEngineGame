#include "RadialBluer.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void RadialBluer::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	//基底クラスの初期化
	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("RadialBluerPSO");
	//Bufferの名前付け
	inputResource_->SetName(L"RadialBluer::inputResource_");
	outputResource_->SetName(L"RadialBluer::outputResource_");

	blurInfoResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(RadialBlurInfo));
	blurInfoResource_->SetName(L"RadialBluer::blurInfoResource_");
	//mapping
	blurInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&radialBlurInfo_));

	radialBlurInfo_->center = Vector2(0.5f, 0.5f);
	radialBlurInfo_->blurWidth = 0.01f;
	radialBlurInfo_->enable = false;
}

void RadialBluer::UpdateImGui() {
#ifdef _DEBUG

	ImGui::Begin("RadialBluer");
	ImGui::SliderFloat2("Center", &radialBlurInfo_->center.x, 0.0f, 1.0f, "%.5f");
	ImGui::SliderFloat("BlurWidth", &radialBlurInfo_->blurWidth, 0.0f, 1.0f, "%.5f");
	ImGui::Checkbox("Enable", &radialBlurInfo_->enable);
	ImGui::End();

#endif // _DEBUG
}

void RadialBluer::DisPatch() {

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
	//blurInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(computePSO_->GetComputeBindResourceIndex("gBlurInfo"), blurInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
