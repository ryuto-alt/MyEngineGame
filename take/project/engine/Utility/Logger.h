#pragma once
#include <Windows.h>
#include <string>

namespace Logger {


	//出力ウィンドウに文字を出す関数
	void Log(const std::string& message);

	//エラーメッセージの取得
	//static std::string GetErrorMessage(HRESULT hr);
};

