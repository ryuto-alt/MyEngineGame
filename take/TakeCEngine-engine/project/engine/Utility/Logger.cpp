#include "Logger.h"

#include <debugapi.h>
#include <iostream>

//出力ウィンドウに文字を出す関数
void Logger::Log(const std::string& message) {
	 OutputDebugStringA(message.c_str());
}


//エラーメッセージの取得
//std::string Logger::GetErrorMessage(HRESULT hr) {
//	LPVOID lpMsgBuf;
//	DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
//
//	FormatMessage(
//		dwFlags,
//		NULL,
//		hr,
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		(LPWSTR)&lpMsgBuf,
//		0, NULL);
//
//	std::string errorMsg = (LPSTR)lpMsgBuf;
//	LocalFree(lpMsgBuf);
//	return errorMsg;
//}
