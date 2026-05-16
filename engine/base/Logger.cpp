#include "Logger.h"

#include <d3d12.h>

#pragma comment (lib, "d3d12.lib")

namespace Logger
{
	void Log(const std::string& message)
	{
		OutputDebugStringA(message.c_str());
	}
}