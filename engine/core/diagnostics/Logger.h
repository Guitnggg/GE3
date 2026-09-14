#pragma once

#include <string>

/// <summary>
/// デバッグ出力用のログ関数群
/// </summary>
namespace Logger
{
	/// <summary>
	/// Visual Studioの出力ウィンドウへ文字列を出力する
	/// </summary>
	void Log(const std::string& message);
}
