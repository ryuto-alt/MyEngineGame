#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "base/WinApp.h"
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <string>

class ImGuiManager {
public:

	ImGuiManager() = default;
	~ImGuiManager() = default;

	void Initialize(WinApp* winApp,DirectXCommon* dxCommon,SrvManager* srvManager);

	void Finalize();

	void Begin();

	void End();

	void DrawDebugScreen();

	void PostDraw();

	static void QuaternionScreenPrintf(const std::string& label, const Quaternion& q);

	static void Vector3ScreenPrintf(const std::string& label, const Vector3& v);

	static void Matrix4x4ScreenPrintf(const std::string& label, const Matrix4x4& m);

	void SetRenderTextureIndex(uint32_t index) {
		renderTextureIndex_ = index;
	}
private:

	
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	uint32_t renderTextureIndex_ = 0; // ImGuiのレンダリングターゲットのSRVインデックス
};

