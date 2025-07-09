#pragma once
#include "Posteffect/PostEffect.h"

struct VignetteInfo {
	float vignetteScale = 0.0f; //中心から外側への減衰の速さ
	float vignettePower = 0.0f; //Vignetteの強さ
	bool flag = false; //Vignetteの有効無効
};

class Vignette : public PostEffect {
public:

	Vignette() = default;
	~Vignette() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) override;
	void UpdateImGui() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void DisPatch() override;

private:

	VignetteInfo* vignetteInfoData_ = nullptr;
	ComPtr<ID3D12Resource> vignetteInfoResource_;
};

