#pragma once
#include "PostEffect/PostEffect.h"
class GrayScale : public PostEffect {
public:
	GrayScale() = default;
	~GrayScale() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
	
	void UpdateImGui() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void DisPatch() override;

private:

	//グレースケールの種類
	enum class GrayScaleType {
		GRAYSCALE,
		SEPIA,
		NONE,
	};
	int32_t* grayScaleTypeData_ = nullptr;
	ComPtr<ID3D12Resource> grayScaleTypeResource_;

};

