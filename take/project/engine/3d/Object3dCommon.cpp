#include "Object3dCommon.h"
#include "DirectXCommon.h"

#include "ImGuiManager.h"
#include "CameraManager.h"
#include "Vector3Math.h"
#include <numbers>
#include <algorithm>

Object3dCommon* Object3dCommon::instance_ = nullptr;

Object3dCommon* Object3dCommon::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new Object3dCommon();
	}
	return instance_;
	
}

void Object3dCommon::Initialize(DirectXCommon* directXCommon) {

	//DirectXCommon取得
	dxCommon_ = directXCommon;
	//PSO生成

	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Object3d.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Object3d.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL);
	pso_->SetGraphicPipelineName("Object3dPSO:graphic");
	pso_->CompileComputeShader(dxCommon_->GetDXC(), L"Skinning.CS.hlsl");
	pso_->CreateComputePSO(dxCommon_->GetDevice());
	pso_->SetComputePipelineName("Object3dPSO:Conpute");
	graphicRootSignature_ = pso_->GetGraphicRootSignature();
	computeRootSignature_ = pso_->GetComputeRootSignature();

#pragma region "Lighting"
	//平行光源用Resourceの作成
	directionalLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLightData));
	directionalLightResource_->SetName(L"Object3dCommon::directionalLightResource_");
	directionalLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-1.0f,0.0f };
	//光源の輝度書き込む
	directionalLightData_->intensity_ = 1.0f;

	//PointLight用のResourceの作成
	pointLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLightData));
	pointLightResource_->SetName(L"Object3dCommon::pointLightResource_");
	pointLightData_ = nullptr;
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
	pointLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData_->position_ = { 0.0f,2.0f,0.0f };
	pointLightData_->intensity_ = 0.0f;
	pointLightData_->radius_ = 10.0f;
	pointLightData_->decay_ = 1.0f;

	//SpotLight用のResourceの作成
	spotLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(SpotLightData));
	spotLightResource_->SetName(L"Object3dCommon::spotLightResource_");
	spotLightData_ = nullptr;
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	spotLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	spotLightData_->position_ = { 2.0f,2.0f,0.0f };
	spotLightData_->direction_ = { -1.0f,-1.0f,0.0f };
	spotLightData_->distance_ = 7.0f;
	spotLightData_->intensity_ = 4.0f;
	spotLightData_->decay_ = 2.0f;
	spotLightData_->cosAngle_ = std::cosf(std::numbers::pi_v<float> / 3.0f);
	spotLightData_->penumbraAngle_ = std::numbers::pi_v<float> / 6.0f;

#pragma endregion
}

void Object3dCommon::UpdateImGui() {
	ImGui::Begin("Lighting");
	ImGui::Text("DirectionalLight");
	ImGui::SliderFloat3("Direction", &directionalLightData_->direction_.x, -1.0f, 1.0f);
	directionalLightData_->direction_ = Vector3Math::Normalize(directionalLightData_->direction_);
	ImGui::DragFloat("dirIntensity", &directionalLightData_->intensity_, 0.01f);
	ImGui::Text("PointLight");
	ImGui::ColorEdit4("Color", &pointLightData_->color_.x);
	ImGui::DragFloat3("Position", &pointLightData_->position_.x, 0.01f);
	ImGui::DragFloat("Intensity", &pointLightData_->intensity_, 0.01f);
	ImGui::SliderFloat("PointLightRadius", &pointLightData_->radius_, 0.0f, 100.0f);
	ImGui::SliderFloat("Decay", &pointLightData_->decay_, 0.0f, 2.0f);
	ImGui::Text("SpotLight");
	ImGui::ColorEdit4("SpotColor", &spotLightData_->color_.x);
	ImGui::DragFloat3("SpotPosition", &spotLightData_->position_.x, 0.01f);
	ImGui::DragFloat3("SpotDirection", &spotLightData_->direction_.x, 0.01f);
	spotLightData_->direction_ = Vector3Math::Normalize(spotLightData_->direction_);
	ImGui::DragFloat("SpotDistance", &spotLightData_->distance_, 0.01f);
	ImGui::DragFloat("SpotIntensity", &spotLightData_->intensity_, 0.01f);
	ImGui::SliderFloat("SpotDecay", &spotLightData_->decay_, 0.0f, 2.0f);
	ImGui::SliderAngle("SpotCosAngle", &spotLightData_->cosAngle_);
	ImGui::SliderAngle("SpotPenumbraAngle", &spotLightData_->penumbraAngle_);
	ImGui::End();
}

void Object3dCommon::Finalize() {
	directionalLightResource_.Reset();
	pointLightResource_.Reset();
	spotLightResource_.Reset();
	graphicRootSignature_.Reset();
	computeRootSignature_.Reset();
	pso_.reset();
	dxCommon_ = nullptr;
	delete instance_;
	instance_ = nullptr;
}

void Object3dCommon::PreDraw() {


	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	// ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicRootSignature_.Get());
	
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SetCBufferViewCamera(pso_.get());
	SetGraphicCBufferViewLghiting(pso_.get());
}

void Object3dCommon::DisPatch() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetComputePipelineState());
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignature_.Get());
}

void Object3dCommon::SetGraphicCBufferViewLghiting(PSO* pso) {
	//DirectionalLight
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(pso->GetGraphicBindResourceIndex("gDirectionalLight"), directionalLightResource_->GetGPUVirtualAddress());
	//pointLightのCBuffer
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(pso->GetGraphicBindResourceIndex("gPointLight"), pointLightResource_->GetGPUVirtualAddress());
	//spotLightのCBuffer
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(pso->GetGraphicBindResourceIndex("gSpotLight"), spotLightResource_->GetGPUVirtualAddress());

}

void Object3dCommon::SetCBufferViewCamera(PSO* pso) {
	//カメラ情報のCBufferの場所を指定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gCamera"), CameraManager::GetInstance()->GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

}
