#include "WinApp.h"

#include <cassert>
#pragma region imgui
#ifdef _DEBUG
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // DEBUG
#pragma endregion

WinApp::WinApp() {}

WinApp::~WinApp() {
#ifdef _DEBUG
	debugController_.Reset();
#endif // _DEBUG

}

//=====================================================================
//			初期化
//=====================================================================

void WinApp::Initialize(const wchar_t title[]) {

	HRESULT hr;
	hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	// ウィンドウプロシージャ
	wc_.lpfnWndProc = WinApp::WindowProc;
	//ウィンドウクラス名()
	wc_.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc_);

	CreateGameWindow(title);

	assert(SUCCEEDED(hr));

#ifdef _DEBUG

	//debugController
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		//デバッグレイヤーを有効化する
		debugController_->EnableDebugLayer();
		//さらにGPU側にもチェックを行うようにする
		debugController_->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG

}

//=====================================================================
//			終了・開放処理
//=====================================================================

void WinApp::Finalize() {

	CloseWindow(hwnd_);
	CoUninitialize();
}

//=======================================================================
//			メッセージの処理
//=======================================================================

bool WinApp::ProcessMessage() {

	MSG msg{};//メッセージ

	//Windowにメッセージが来てたら最優先で処理させる
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 終了メッセージが来たらループを抜ける
	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}

//=======================================================================
//			ビューポートの取得
//=======================================================================

D3D12_VIEWPORT WinApp::GetViewport() const {
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = offsetX_;
	viewport.TopLeftY = offsetY_;
	viewport.Width = static_cast<float>(kScreenWidth);
	viewport.Height = static_cast<float>(kScreenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	return viewport;
}

//=======================================================================
//			ウィンドウのクライアント領域のサイズを取得
//=======================================================================

D3D12_RECT WinApp::GetScissorRect() const {
	
	D3D12_RECT scissorRect = {};
	scissorRect.left = static_cast<LONG>(offsetX_);
	scissorRect.top = static_cast<LONG>(offsetY_);
	scissorRect.right = static_cast<LONG>(kScreenWidth + offsetX_);
	scissorRect.bottom = static_cast<LONG>(kScreenHeight + offsetY_);
	return scissorRect;
}

//=======================================================================
//ウィンドウプロシージャ
//=======================================================================

LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
#ifdef _DEBUG
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return true;
	}
#endif // _DEBUG


	//メッセージに応じてゲーム特有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//=======================================================================
//			ゲームウィンドウの作成
//=======================================================================

void WinApp::CreateGameWindow(const wchar_t title[]) {

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	wrc_ = { 0,0,kWindowWidth,kWindowHeight };

	//クライアント領域を元に実際のサイズにwrc_を変更してもらう
	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(
		wc_.lpszClassName,       //利用するクラス名
		title,                 //タイトルバーの文字(何でもいい,引数に入れることもできる)
		WS_OVERLAPPEDWINDOW,    //よく見るウィンドウスタイル
		CW_USEDEFAULT,          //表示X座標(Windowsに任せる)
		CW_USEDEFAULT,          //表示Y座標(WindowsOSに任せる)
		wrc_.right - wrc_.left,   //ウィンドウ横幅
		wrc_.bottom - wrc_.top,   //ウィンドウ縦幅
		nullptr,                //親ウィンドウハンドル
		nullptr,                //メニューハンドル
		wc_.hInstance,           //インスタンスハンドル
		nullptr                 //オプション
	);

	//ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);
}