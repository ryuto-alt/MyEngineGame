#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class WinApp {
public:

// ゲームスクリーンのサイズ
#ifdef _DEBUG
static const int32_t kScreenWidth = 1024;
static const int32_t kScreenHeight = 600;
// クライアント領域のサイズ（デバッグ用）
static const int32_t kWindowWidth = 1880;
static const int32_t kWindowHeight = 920;
#else
static const int32_t kScreenWidth = 1880;
static const int32_t kScreenHeight = 920;
// クライアント領域のサイズ（リリース用）
static const int32_t kWindowWidth = 1880;
static const int32_t kWindowHeight = 920;
#endif
	WinApp();
	~WinApp();

	/// <summary>
	/// ウィンドウプロシージャ
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="msg">メッセージ番号</param>
	/// <param name="wparam">メッセージ情報1</param>
	/// <param name="lparam">メッセージ情報2</param>
	/// <returns>成否</returns>
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,WPARAM wParam, LPARAM lParam);

	/// <summary>
	/// ゲームウィンドウの作成
	/// </summary>
	void CreateGameWindow(const wchar_t title[]);

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const wchar_t title[]);

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// メッセージの処理
	/// </summary>
	/// <returns>終了かどうか</returns>
	bool ProcessMessage();

public:

	/// <summary>
	/// ウィンドウクラスのhInstance取得
	/// </summary>
	/// <returns></returns>
	const HINSTANCE& GetHInstance() const { return wc_.hInstance; }

	/// <summary>
	/// ウィンドウハンドルの取得
	/// </summary>
	/// <returns></returns>
	const HWND& GetHwnd() const { return hwnd_; }

	const RECT& GetWindowRect() const { return wrc_; }

	const float& GetOffsetX() const { return offsetX_; }

	const float& GetOffsetY() const { return offsetY_; }

	/// <summary>
	/// ビューポートの取得
	/// </summary>
	D3D12_VIEWPORT GetViewport() const;

	/// <summary>
	/// ウィンドウのクライアント領域のサイズを取得
	/// </summary>
	D3D12_RECT GetScissorRect() const;

private:

	//ウィンドウクラス
	WNDCLASS wc_;

	RECT wrc_;
	//ウィンドウハンドル
	HWND hwnd_;
	//debugController
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_;
	
	//描画位置のオフセット
	float offsetX_ = 0.0f;
	float offsetY_ = 0.0f;
};