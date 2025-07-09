#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/PipelineStateObject.h"
#include "PostEffect/PostEffect.h"
#include <string>
#include <vector>

enum IntermediateResourceType {
	FRONT,
	BACK,
};

struct NamedPostEffect {
	std::string name;
	std::unique_ptr<PostEffect> postEffect;
};

class PostEffectManager {
public:
	PostEffectManager() = default;
	~PostEffectManager() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	void UpdateImGui();

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	///	描画処理
	/// </summary>
	void Draw(PSO* pso);

	/// <summary>
	/// 全PostEffectのCSによる処理
	/// </summary>
	void AllDispatch();

	void ApplyEffect(const std::string& name);

	void InitializeEffect(const std::string& name, const std::wstring& csFilePath);

	void SetRenderTextureResource(ComPtr<ID3D12Resource> renderTextureResource) {
		renderTextureResource_ = renderTextureResource;
	}

	void SetRenderTextureSrvIndex(uint32_t srvIndex) {
		renderTextureSrvIndex_ = srvIndex;
	}

	uint32_t GetFinalOutputSrvIndex() const;

private:

	DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ

	//srv/uavとして利用する中間リソース
	ComPtr<ID3D12Resource> intermediateResource_[2];
	uint32_t srvIndex_[2] = {};
	uint32_t uavIndex_[2] = {};

	ComPtr<ID3D12Resource> renderTextureResource_; 
	uint32_t renderTextureSrvIndex_ = 0;
	uint32_t finalOutputSrvIndex_ = 0;

	// Ping-Pongバッファの切り替えフラグ
	bool currentWriteBufferIsA_ = true; 

	//postEffectのコンテナ
	std::vector<NamedPostEffect> postEffects_;
};