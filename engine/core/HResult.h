#pragma once

#include <Windows.h>

#include <string_view>

namespace HResult {
	/// <summary>
	/// HRESULT失敗時に操作名と16進数コードをログへ記録し、例外を送出する。
	/// </summary>
	void ThrowIfFailed(HRESULT result, std::string_view operation);
}
