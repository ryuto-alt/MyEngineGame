#include "WinApp.h"

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef _DEBUG
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize()
{
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
#pragma region ウィンドウクラスの登録


	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"CG2WindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);
#pragma endregion

#pragma region ウィンドウサイズを決める

	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
#pragma endregion

#pragma region ウィンドウ生成と表示
	hwnd = CreateWindow(
		wc.lpszClassName,
		L"UnoEngineGame",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);
#pragma endregion

	ShowWindow(hwnd, SW_SHOW);


}

void WinApp::Update()
{
}

void WinApp::Finalize()
{
	CloseWindow(hwnd);
	CoUninitialize();
}

bool WinApp::ProcessMessage()
{
	MSG msg{};
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	if (msg.message==WM_QUIT) {
		return true;
	}

	return false;
}

void WinApp::ToggleFullscreen()
{
	if (!isFullscreen_) {
		// ウィンドウモード → フルスクリーン
		// 現在のウィンドウスタイルと位置を保存
		windowedStyle_ = GetWindowLong(hwnd, GWL_STYLE);
		GetWindowRect(hwnd, &windowedRect_);

		// ウィンドウスタイルを変更（枠なし）
		SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);

		// フルスクリーンサイズに変更
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_FRAMECHANGED);

		isFullscreen_ = true;
	}
	else {
		// フルスクリーン → ウィンドウモード
		// 元のウィンドウスタイルに戻す
		SetWindowLong(hwnd, GWL_STYLE, windowedStyle_);

		// 元のサイズと位置に戻す
		SetWindowPos(hwnd, HWND_TOP,
			windowedRect_.left,
			windowedRect_.top,
			windowedRect_.right - windowedRect_.left,
			windowedRect_.bottom - windowedRect_.top,
			SWP_FRAMECHANGED);

		isFullscreen_ = false;
	}
}
