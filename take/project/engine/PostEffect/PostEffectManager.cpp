#include "PostEffectManager.h"
#include "engine/base/WinApp.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/Logger.h"
#include "PostEffect/Dissolve.h"
#include "PostEffect/Vignette.h"
#include "PostEffect/GrayScale.h"
#include "PostEffect/BoxFilter.h"
#include "PostEffect/RadialBluer.h"
#include "PostEffect/LuminanceBasedOutline.h"

//====================================================================
//	初期化
//====================================================================

void PostEffectManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//中間リソースの生成
	intermediateResource_[FRONT] = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(), WinApp::kScreenWidth, WinApp::kScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	intermediateResource_[FRONT]->SetName(L"intermediateResource_A");
	intermediateResource_[BACK] = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(), WinApp::kScreenWidth, WinApp::kScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	intermediateResource_[BACK]->SetName(L"intermediateResource_B");
}

void PostEffectManager::UpdateImGui() {

	for (auto& postEffect : postEffects_) {
		postEffect.postEffect->UpdateImGui();
	}
}

//====================================================================
// 終了処理
//====================================================================

void PostEffectManager::Finalize() {
	intermediateResource_[FRONT].Reset();
	intermediateResource_[BACK].Reset();
	renderTextureResource_.Reset();
	postEffects_.clear();
}

//======================================================================
// ComputeShaderによる全PostEffectの処理
//======================================================================

void PostEffectManager::AllDispatch() {

	// RENDER_TARGET >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,         //stateBefore
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, //stateAfter
		renderTextureResource_.Get());              //currentResource

	for (auto& postEffect : postEffects_) {
		postEffect.postEffect->DisPatch();
	}

	//NON_PIXCEL_SHADER_RESOURCE >> RENDER_TARGET
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		renderTextureResource_.Get());
}

//======================================================================
// 描画処理
//======================================================================

void PostEffectManager::Draw(PSO* pso) {

	// gTexture
	srvManager_->SetGraphicsRootDescriptorTable(
		pso->GetGraphicBindResourceIndex("gTexture"),postEffects_.back().postEffect->GetOutputTextureSrvIndex());
}

void PostEffectManager::ApplyEffect(const std::string& name) {

	// 読み込み済みかどうか検索
	for (const auto& postEffect : postEffects_) {
		if (postEffect.name == name) {
			postEffect.postEffect->DisPatch();
			return;
		}
	}
}

void PostEffectManager::InitializeEffect(const std::string& name, const std::wstring& csFilePath) {

	// 読み込み済みかどうか検索
	for (const auto& postEffect : postEffects_) {
		if (postEffect.name == name) {
			Logger::Log("PostEffectManager::InitializeEffect() : PostEffect is already initialized.");
			return;
		}
	}

	std::unique_ptr<PostEffect> postEffect;

	// 入出力テクスチャを決定（Ping-Pong切り替え）
	ComPtr<ID3D12Resource> inputResource = nullptr;
	uint32_t inputSrvIndex = 0;
	ComPtr<ID3D12Resource> outputResource = nullptr;

	// PostEffectの初期化
	if (name == "GrayScale") {
		postEffect = std::make_unique<GrayScale>();
	} else if (name == "Vignette") {
		postEffect = std::make_unique<Vignette>();
	} else if (name == "BoxFilter") {
		postEffect = std::make_unique<BoxFilter>();
	} else if (name == "RadialBluer") {
		postEffect = std::make_unique<RadialBluer>();
	} else if (name == "Dissolve") {
		postEffect = std::make_unique<Dissolve>();
	} else if (name == "LuminanceBasedOutline") {
		postEffect = std::make_unique<LuminanceBasedOutline>();
	} else {
		Logger::Log("PostEffectManager::InitializeEffect() : PostEffect is not found.");
		return;
	}

	if (postEffects_.empty()) {

		// 最初の入力は通常の描画結果（renderTextureResource_）
		inputResource = renderTextureResource_;
		inputSrvIndex = renderTextureSrvIndex_;
	}
	else {
		// 直前の出力を次の入力に
		const auto& prevEffect = postEffects_.back().postEffect;
		inputResource = prevEffect->GetOutputTextureResource();
		inputSrvIndex = prevEffect->GetOutputTextureSrvIndex();
	}

	//ping-pong状態で出力リソースとインデックスを決定
	IntermediateResourceType bufferIndex = currentWriteBufferIsA_ ? FRONT : BACK;
	outputResource = intermediateResource_[bufferIndex];

	// PostEffectの初期化（Ping-Pongバッファ指定）
	postEffect->Initialize(dxCommon_, srvManager_, csFilePath,
		inputResource, inputSrvIndex,outputResource);


	// PostEffectのコンテナに追加
	postEffects_.push_back({ name, std::move(postEffect) });
	// Ping-Pongバッファの切り替え
	currentWriteBufferIsA_ = !currentWriteBufferIsA_; 
}

uint32_t PostEffectManager::GetFinalOutputSrvIndex() const {
	return postEffects_.back().postEffect->GetOutputTextureSrvIndex();
}
