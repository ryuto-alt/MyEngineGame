#pragma once
#include "DirectXCommon.h"
class GraphicsPipeline
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);


	void Create();//3dオブジェクト用
	void CreateParticle();//パーティクル用
	void CreateSprite();//スプライト用
	void CreateCopyImage();//コピーイメージ用
	void CreateLine();//ライン用
	void CreateSkinning();//スキニング用



	//ルートシグネチャの作成
	void RootSignatureCreate();//3dオブジェクト用
	void RootSignatureParticleCreate();//パーティクル用
	void RootSignatureSpriteCreate();//スプライト用
	void RootSignatureCopyImageCreate();//コピーイメージ用
	void RootSignatureLineCreate();//ライン用
	void RootSignatureSkinningCreate();//スキニング用



	//ゲッター
	ID3D12RootSignature* GetRootSignature()const { return rootSignature.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineState()const { return graphicsPipelineState.Get(); }
	//パーティクル用のPSO
	ID3D12RootSignature* GetRootSignatureParticle()const { return rootSignatureParticle.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateParticle()const { return graphicsPipelineStateParticle.Get(); }

	//スプライト用のPSO
	ID3D12RootSignature* GetRootSignatureSprite()const { return rootSignatureSprite.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateSprite()const { return graphicsPipelineStateSprite.Get(); }

	//コピーイメージ用のPSO
	ID3D12RootSignature* GetRootSignatureCopyImage()const { return rootSignatureCopyImage.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateCopyImage()const { return graphicsPipelineStateCopyImage.Get(); }

	//ライン用のPSO
	ID3D12RootSignature* GetRootSignatureLine()const { return rootSignatureLine.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateLine()const { return graphicsPipelineStateLine.Get(); }

	//スキニング用のPSO
	ID3D12RootSignature* GetRootSignatureSkinning()const { return rootSignature.Get(); }
	ID3D12PipelineState* GetGraphicsPipelineStateSkinning()const { return graphicsPipelineState.Get(); }

private:
	DirectXCommon* dxCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	//パーティクル用のルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureParticle = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateParticle = nullptr;


	//スプライト用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureSprite = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateSprite = nullptr;


	//コピーイメージ用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureCopyImage = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateCopyImage = nullptr;


	//ライン用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureLine = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateLine = nullptr;


};

