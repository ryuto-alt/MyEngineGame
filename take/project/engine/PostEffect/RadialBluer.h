#pragma once
#include "PostEffect/PostEffect.h"

struct RadialBlurInfo {
	Vector2 center; // 中心UV座標
	float blurWidth; // ブラーの幅
	bool enable; // ブラーの有効フラグ
};

class RadialBluer : public PostEffect {
public:
	RadialBluer() = default;
	~RadialBluer() = default;
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

	//ブラーの情報
	RadialBlurInfo* radialBlurInfo_;
	//ブラーの情報を格納するバッファ
	ComPtr<ID3D12Resource> blurInfoResource_ = nullptr;

};