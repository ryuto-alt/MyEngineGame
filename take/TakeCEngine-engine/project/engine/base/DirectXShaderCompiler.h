#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>

class DXC {

	//////////////////////////////////////////////////////////////////////////////////////////
	///			dxc
	//////////////////////////////////////////////////////////////////////////////////////////

public:

	DXC() = default;
	~DXC();

	/// <summary>
	/// dxc初期化
	/// </summary>
	void InitializeDxc();

	/// <summary>
	/// 開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// ShaderCompile関数
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler
	);

	/// <summary>
	/// dxcUtilsの取得
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcUtils> GetDxcUtils() { return dxcUtils_; }

	/// <summary>
	/// dxcCompilerの取得
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcCompiler3> GetDxcCompiler() { return dxcCompiler_; }

	/// <summary>
	/// includeHandlerの取得
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> GetIncludeHandler() { return includeHandler_; }

private:
	
	//メンバ変数
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource_;
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult_;
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError_;
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob_;
};

