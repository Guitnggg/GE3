#include "Logger.h"

#include <Windows.h>

namespace Logger {
// Visual Studioの出力ウィンドウへメッセージを出力する
void Log(const std::string &message) {
	OutputDebugStringA(message.c_str());
}
} // namespace Logger
