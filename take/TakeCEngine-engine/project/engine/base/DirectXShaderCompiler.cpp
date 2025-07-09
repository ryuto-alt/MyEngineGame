#include "DirectXShaderCompiler.h"
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include <cassert>
#include <format>

#pragma comment(lib,"dxcompiler.lib")

////////////////////////////////////////////////////////////////////////////////////////////////////
///			DXC
////////////////////////////////////////////////////////////////////////////////////////////////////

DXC::~DXC() {
	Finalize();
}

void DXC::InitializeDxc() {

	HRESULT result = S_FALSE;

	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(result));
	result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(result));

	includeHandler_ = nullptr;
	result = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(result));
}

void DXC::Finalize() {

	shaderBlob_.Reset();
	if (shaderError_) {
		shaderError_.Reset();
	}
	shaderResult_.Reset();
	shaderSource_.Reset();
	includeHandler_.Reset();
	dxcCompiler_.Reset();
	dxcUtils_.Reset();
}

Microsoft::WRL::ComPtr<IDxcBlob> DXC::CompileShader(
	const std::wstring& filePath, const wchar_t* profile,
	IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler) {


	HRESULT result = S_FALSE;

	std::wstring fullPath = L"Resources/shaders/" + filePath;

	//これからシェーダーをコンパイルする旨をログに出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader, path:{},profile{}\n", fullPath, profile)));
	//hlslファイルを読み込む
	shaderSource_ = nullptr;
	result = dxcUtils->LoadFile(fullPath.c_str(), nullptr, &shaderSource_);
	//読めなかったら止める
	if(FAILED(result)) {
		Logger::Log(StringUtility::ConvertString(std::format(L"Failed to load file, path:{}\n", fullPath)));
		assert(false);
	}

	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource_->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource_->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; //UTF8の文字コードであることを通知

	LPCWSTR arguments[] = {
		filePath.c_str(), //コンパイル対象のhlslファイル名
		L"-E",L"main", //エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",profile, //ShaderProfileの設定
		L"-Zi",L"-Qembed_debug", //デバッグ用の情報を埋め込む
		L"-Od", //最適化を外しておく
		L"-Zpr", //メモリレイアウトは行優先
		L"-I", L"Resources/shaders",
	};

	//実際にShaderをコンパイルする
	shaderResult_ = nullptr;
	result = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult_)
	);
	//コンパイルエラーではなくdxcが起動できないほど致命的な状況
	assert(SUCCEEDED(result));

	//警告・エラーが出てたらログに出して止める
	shaderError_ = nullptr;
	shaderResult_->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError_), nullptr);
	if (shaderError_ != nullptr && shaderError_->GetStringLength() != 0) {
		Logger::Log(shaderError_->GetStringPointer());
		//警告・エラーダメ絶対
		assert(false);
	}

	//コンパイル結果から実行用のバイナリ部分を取得
	shaderBlob_ = nullptr;
	result = shaderResult_->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob_), nullptr);
	assert(SUCCEEDED(result));
	//成功したログを出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	//もう使わないリソースを開放する
	shaderSource_.Reset();
	shaderResult_.Reset();

	//実行用のバイナリを返却
	return shaderBlob_;
}