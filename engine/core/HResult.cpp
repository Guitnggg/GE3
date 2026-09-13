#include "HResult.h"

#include <format>
#include <stdexcept>
#include <string>

#include "Logger.h"

void HResult::ThrowIfFailed(HRESULT result, std::string_view operation) {
	// 成功コードではログや例外を発生させず、呼び出し元の処理を継続する
	if (SUCCEEDED(result)) {
		return;
	}

	// 操作名と16進数のHRESULTを組み合わせ、原因を追跡できるメッセージを作る
	const std::string message = operation.starts_with("Failed")
		? std::format("{} (HRESULT 0x{:08X})", operation, static_cast<unsigned long>(result))
		: std::format("{} failed (HRESULT 0x{:08X}).", operation, static_cast<unsigned long>(result));
	// デバッガ出力へ記録してから例外化し、Releaseでも失敗を見逃さないようにする
	Logger::Log(message + "\n");
	throw std::runtime_error(message);
}
