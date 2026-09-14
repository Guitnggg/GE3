#pragma once

#include <string>

/// <summary>
/// 文字列変換用のユーティリティ関数群
/// </summary>
namespace StringUtility
{
	/// <summary>
	/// UTF-8文字列をワイド文字列に変換する
	/// </summary>
	std::wstring ConvertString(const std::string& str);

	/// <summary>
	/// ワイド文字列をUTF-8文字列に変換する
	/// </summary>
	std::string ConvertString(const std::wstring& str);
};
