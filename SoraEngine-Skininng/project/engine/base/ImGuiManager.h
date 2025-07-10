#pragma once
#include "DirectXCommon.h"
#include "WinApp.h"
class ImGuiManager
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, WinApp* winapp);

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// Imgui受付開始
	/// </summary>
	void Begin();
	
	/// <summary>
	/// Imgui受付終了
	/// </summary>
	void End();

	/// <summary>
	///画面への描画
	/// </summary>
	void Draw();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>srvHeap_;

	DirectXCommon* dxCommon_ = nullptr;
	WinApp* winapp_ = nullptr;
};

