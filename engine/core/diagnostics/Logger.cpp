#include "Logger.h"

#include <Windows.h>
#include <fstream>
#include <mutex>

namespace Logger {
// Visual Studioの出力ウィンドウと実行ディレクトリのログへメッセージを出力する。
void Log(const std::string &message) {
	OutputDebugStringA(message.c_str());
	static std::mutex mutex;
	const std::scoped_lock lock(mutex);
	std::ofstream file("MadeEngine.log", std::ios::app);
	if (file) {
		file << message;
	}
}
} // namespace Logger
