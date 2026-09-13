#include "HResult.h"

#include <format>
#include <stdexcept>
#include <string>

#include "Logger.h"

void HResult::ThrowIfFailed(HRESULT result, std::string_view operation) {
	if (SUCCEEDED(result)) {
		return;
	}

	const std::string message = operation.starts_with("Failed")
		? std::format("{} (HRESULT 0x{:08X})", operation, static_cast<unsigned long>(result))
		: std::format("{} failed (HRESULT 0x{:08X}).", operation, static_cast<unsigned long>(result));
	Logger::Log(message + "\n");
	throw std::runtime_error(message);
}
