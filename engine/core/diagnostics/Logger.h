#pragma once

#include <string>

/// <summary>
/// エンジンの診断文字列をデバッガへ送るログ関数群。
/// </summary>
namespace Logger {
/// <summary>
/// Visual Studioの出力ウィンドウへ文字列を出力する
/// </summary>
void Log(const std::string &message);
} // namespace Logger
