#include "SpriteCommon.h"
#include "Logger.h"

SpriteCommon* SpriteCommon::instance_ = nullptr;
SpriteCommon* SpriteCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new SpriteCommon();
	}
	return instance_;
}

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateSprite();

}

void SpriteCommon::Finalize()
{

	delete instance_;
	instance_ = nullptr;

}

void SpriteCommon::CommonDraw()
{
	//RootSignatureを設定。POSに設定しているけどベット設定が必要
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureSprite());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateSprite());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}






