#pragma once

#include <Windows.h>

#include <string_view>

namespace HResult {
	/// <summary>
	/// HRESULT失敗時に操作名と16進数コードをログへ記録し、例外を送出する。
	/// 成功を表すHRESULTの場合は何も行わず、そのまま処理を継続する。
	/// </summary>
	/// <param name="result">WindowsまたはDirectX APIから返されたHRESULT</param>
	/// <param name="operation">ログへ表示する実行中の処理名</param>
	void ThrowIfFailed(HRESULT result, std::string_view operation);
}
