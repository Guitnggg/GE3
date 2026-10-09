#pragma once

#include <string>

/// <summary>
/// UTF-8文字列とWindowsワイド文字列を相互変換する関数群。
/// </summary>
namespace StringUtility {
/// <summary>
/// UTF-8文字列をワイド文字列に変換する
/// </summary>
std::wstring ConvertString(const std::string &str);

/// <summary>
/// ワイド文字列をUTF-8文字列に変換する
/// </summary>
std::string ConvertString(const std::wstring &str);
}; // namespace StringUtility
