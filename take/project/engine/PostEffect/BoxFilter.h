#pragma once
#include "PostEffect/PostEffect.h"
class BoxFilter : public PostEffect {
public:
	BoxFilter() = default;
	~BoxFilter() = default;
	void Initialize(
		DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
	void UpdateImGui() override;
	
	void DisPatch() override;

private:

};

