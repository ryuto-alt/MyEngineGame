#include "SpriteCommon.h"
#include "DirectXCommon.h"
#include "PipelineStateObject.h"

SpriteCommon* SpriteCommon::instance_ = nullptr;

SpriteCommon* SpriteCommon::GetInstance() {	
	if(instance_ == nullptr) {
		instance_ = new SpriteCommon();
	}
	return instance_;
}

void SpriteCommon::Initialize(DirectXCommon* directXCommon) {

	dxCommon_ = directXCommon;

	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Sprite.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Sprite.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ZERO);
	pso_->SetGraphicPipelineName("SpritePSO");
	rootSignature_ = pso_->GetGraphicRootSignature();
}

void SpriteCommon::Finalize() {
	rootSignature_.Reset();
	pso_.reset();
	dxCommon_ = nullptr;
	delete instance_;
	instance_ = nullptr;
}

void SpriteCommon::PreDraw() {

	
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

