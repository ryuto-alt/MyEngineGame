#include "ParticleCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

#include "CameraManager.h"

ParticleCommon* ParticleCommon::instance_ = nullptr;

ParticleCommon* ParticleCommon::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ParticleCommon();
	}
	return instance_;
}

void ParticleCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;

	srvManager_ = srvManager;

	graphicPso_ = std::make_unique<PSO>();
	graphicPso_->CompileVertexShader(dxCommon_->GetDXC(), L"Particle.VS.hlsl");
	graphicPso_->CompilePixelShader(dxCommon_->GetDXC(), L"Particle.PS.hlsl");
	graphicPso_->CreateGraphicPSO(dxCommon_->GetDevice(),D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ZERO, PSO::BlendState::ADD);
	graphicPso_->SetGraphicPipelineName("ParticlePSO");
	graphicRootSignature_ = graphicPso_->GetGraphicRootSignature();

	graphicPsoForGPUParticle_ = std::make_unique<PSO>();
	graphicPsoForGPUParticle_->CompileVertexShader(dxCommon_->GetDXC(), L"GPUParticle.VS.hlsl");
	graphicPsoForGPUParticle_->CompilePixelShader(dxCommon_->GetDXC(), L"GPUParticle.PS.hlsl");
	graphicPsoForGPUParticle_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ZERO,PSO::BlendState::ADD);
	graphicPsoForGPUParticle_->SetGraphicPipelineName("GPUParticlePSO");
	graphicRootSignatureForGPUParticle_ = graphicPsoForGPUParticle_->GetGraphicRootSignature();

	//particle初期化用PSO
	computePsoForGPUParticle_ = std::make_unique<PSO>();
	computePsoForGPUParticle_->CompileComputeShader(dxCommon_->GetDXC(), L"InitializeParticle.CS.hlsl");
	computePsoForGPUParticle_->CreateComputePSO(dxCommon_->GetDevice());
	computePsoForGPUParticle_->SetComputePipelineName("InitializeParticlePSO");
	computeRootSignatureForGPUParticle_ = computePsoForGPUParticle_->GetComputeRootSignature();

	//update用PSO
	psoUpdateParticle_ = std::make_unique<PSO>();
	psoUpdateParticle_->CompileComputeShader(dxCommon_->GetDXC(), L"UpdateParticle.CS.hlsl");
	psoUpdateParticle_->CreateComputePSO(dxCommon_->GetDevice());
	psoUpdateParticle_->SetComputePipelineName("UpdateParticlePSO");
	rootSignatureUpdateParticle_ = psoUpdateParticle_->GetComputeRootSignature();
}

void ParticleCommon::UpdateImGui() {
	//graphicPso_->UpdateImGui();
	graphicPsoForGPUParticle_->UpdateImGui();
}

void ParticleCommon::Finalize() {

	graphicRootSignature_.Reset();
	graphicPso_.reset();

	graphicRootSignatureForGPUParticle_.Reset();
	graphicPsoForGPUParticle_.reset();

	computeRootSignatureForGPUParticle_.Reset();
	computePsoForGPUParticle_.reset();

	rootSignatureUpdateParticle_.Reset();
	psoUpdateParticle_.reset();

	/*srvManager_ = nullptr;
	dxCommon_ = nullptr;*/
}

void ParticleCommon::PreDraw() {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicRootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicPso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleCommon::PreDrawForGPUParticle() {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicRootSignatureForGPUParticle_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicPsoForGPUParticle_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleCommon::DispatchForGPUParticle() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(computePsoForGPUParticle_->GetComputePipelineState());

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(computeRootSignatureForGPUParticle_.Get());
}

void ParticleCommon::DispatchUpdateParticle() {

	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(psoUpdateParticle_->GetComputePipelineState());

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignatureUpdateParticle_.Get());
}
