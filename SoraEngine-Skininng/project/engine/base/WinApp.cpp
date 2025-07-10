#include "WinApp.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#pragma comment(lib,"winmm.lib")



LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {

		return true;

	}
	//メッセージに応じて固有の処理を行う
	switch (msg) {

		//ウィンドウが破壊されたら
	case WM_DESTROY:
		//OS対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}




void WinApp::Initialize()
{
	timeBeginPeriod(1);

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	//ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラス登録する
	RegisterClass(&wc);

	

	RECT wrc = { 0,0,kClientWindth ,kClientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウ生成
	 hwnd = CreateWindow(

		wc.lpszClassName, L"CG2,",
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
	ShowWindow(hwnd, SW_SHOW);


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
		DispatchMessage(&msg);

	}
	if (msg.message == WM_QUIT) {

		return true;
	}
	return false;
}
