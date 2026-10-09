#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <string_view>

namespace HResult {
/// <summary>
/// HRESULTが失敗を示す場合に操作名とコードを記録して例外へ変換する。
/// </summary>
/// <param name="result">WindowsまたはDirectX APIから返されたHRESULT</param>
/// <param name="operation">ログへ表示する実行中の処理名</param>
void ThrowIfFailed(HRESULT result, std::string_view operation);
} // namespace HResult
