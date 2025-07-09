#pragma once
#include <string>

namespace StringUtility {

	//コンバートストリング(wstring型)
	std::wstring ConvertString(const std::string& str);
	//コンバートストリング(string型)
	std::string ConvertString(const std::wstring& str);
}