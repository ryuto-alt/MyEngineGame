#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/RtvManager.h"
#include "base/PipelineStateObject.h"
#include <string>



class PostEffect {
public:
	PostEffect() = default;
	virtual ~PostEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource);

	virtual void UpdateImGui();
	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void DisPatch();

public:

	//inputRenderTextureのSRVインデックスを取得
	uint32_t GetInputTextureSrvIndex() const { return inputTexSrvIndex_; }
	//outputRenderTextureのUAVインデックスを取得
	uint32_t GetOutputTextureUavIndex() const { return outputTexUavIndex_; }
	//outputRenderTextureのSRVインデックスを取得
	uint32_t GetOutputTextureSrvIndex() const { return outputTexSrvIndex_; }
	//inputRenderTextureのポインタを取得
	ComPtr<ID3D12Resource> GetInputTextureResource() const { return inputResource_; }
	//outputRenderTextureのポインタを取得
	ComPtr<ID3D12Resource> GetOutputTextureResource() const { return outputResource_; }

protected:

	DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ

	//RenderTextureリソース
	ComPtr<ID3D12Resource> inputResource_;
	ComPtr<ID3D12Resource> outputResource_;
	uint32_t inputTexSrvIndex_ = 0;
	uint32_t outputTexUavIndex_ = 0;
	uint32_t outputTexSrvIndex_ = 0;

	//computeパイプライン
	std::unique_ptr<PSO> computePSO_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	std::wstring csFilePath_ = L"";
};