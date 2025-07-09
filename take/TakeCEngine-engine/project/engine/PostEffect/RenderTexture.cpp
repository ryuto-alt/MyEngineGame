#include "RenderTexture.h"
#include "engine/base/WinApp.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "PostEffect/PostEffectManager.h"

RenderTexture::~RenderTexture() {
	rootSignature_.Reset();
	renderTexturePSO_.reset();
	renderTextureResource_.Reset();

}

//======================================================================
// 初期化
//======================================================================

void RenderTexture::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, PostEffectManager* postEffectManager ){

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	//RTVManagerの取得
	rtvManager_ = dxCommon_->GetRtvManager();
	postEffectManager_ = postEffectManager;

	//rtvDescの初期化
	rtvDesc_ = {};
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


	//RenderTextureResourceの生成
	renderTextureResource_ = dxCommon_->CreateRenderTextureResource(
		dxCommon_->GetDevice(),WinApp::kScreenWidth,WinApp::kScreenHeight,DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,kRenderTargetClearColor_);
	renderTextureResource_->SetName(L"renderTextureResource_front");

	//RTVの生成
	rtvIndex_ = rtvManager_->Allocate();
	rtvManager_->CreateRTV(renderTextureResource_.Get(), rtvIndex_);

	//SRVの生成
	srvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(renderTextureResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvIndex_);

	//RTVのハンドルの取得
	rtvHandle_ = rtvManager_->GetRtvDescriptorHandleCPU(rtvIndex_);
	//DSVHandleの設定
	dsvHandle_ = dxCommon_->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	//PSO初期化
	renderTexturePSO_ = std::make_unique<PSO>();
	renderTexturePSO_->CompileVertexShader(dxCommon_->GetDXC(), L"PostEffect/FullScreen.VS.hlsl");
	renderTexturePSO_->CompilePixelShader(dxCommon_->GetDXC(), L"PostEffect/CopyImage.PS.hlsl");
	renderTexturePSO_->CreateRenderTexturePSO(dxCommon_->GetDevice());
	renderTexturePSO_->SetGraphicPipelineName("RenderTexturePSO");
	rootSignature_ = renderTexturePSO_->GetGraphicRootSignature();

	//postEffectManagerにRenderTextureReosurceを渡す
	postEffectManager_->SetRenderTextureResource(renderTextureResource_);
	postEffectManager_->SetRenderTextureSrvIndex(srvIndex_);
}

//=======================================================================
// レンダーターターゲットのクリア
//=======================================================================

void RenderTexture::ClearRenderTarget() {



	//描画先のRTVとDSVを設定する
	dxCommon_->GetCommandList()->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);
	// 全画面クリア
	dxCommon_->GetCommandList()->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);
	//指定した深度で画面全体をクリアにする
	dxCommon_->GetCommandList()->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// Viewportを設定
	dxCommon_->GetCommandList()->RSSetViewports(1, &dxCommon_->GetViewport());
	// Scissorの設定
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &dxCommon_->GetScissorRect());
}

//========================================================================
// 描画前処理
//========================================================================

void RenderTexture::PreDraw() {

	ClearRenderTarget();
}

//=======================================================================
// 描画
//========================================================================

void RenderTexture::Draw() {

	// RENDER_TARGET >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,         //stateBefore
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //stateAfter
		renderTextureResource_.Get());              //currentResource

	// ルートシグネチャの設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	// PSOの設定
	dxCommon_->GetCommandList()->SetPipelineState(renderTexturePSO_->GetGraphicPipelineState());
	// プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ポストエフェクトのリソース
	postEffectManager_->Draw(renderTexturePSO_.get());
	// 描画コマンドを発行
	dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

//=======================================================================
// 描画後処理
//========================================================================

void RenderTexture::PostDraw() {


	//PIXCEL_SHADER_RESOURCE >> GENERIC_READ
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());
}