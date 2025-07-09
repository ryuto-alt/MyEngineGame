#pragma once
#include "PostEffect/PostEffect.h"

struct DissolveInfo {
	float threshold;
	bool isDissolve;
	float padding[2];
};

class Dissolve : public PostEffect {
public:
	Dissolve() = default;
	~Dissolve() = default;
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
	void UpdateImGui() override;

	void DisPatch() override;

private:

	ComPtr<ID3D12Resource> dissolveInfoResource_ = nullptr;
	DissolveInfo* dissolveInfoData_ = nullptr;

	std::string maskTextureFilePath_ = "";
};

