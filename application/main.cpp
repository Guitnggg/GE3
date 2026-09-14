#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cstdlib>
#include <exception>
#include <string>

#include "application/MyGame.h"
#include "engine/core/diagnostics/Logger.h"

#pragma comment(lib, "dxcompiler.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	try {
		MyGame game;

		// ゲームの初期化
		game.Initialize();

		while (true) {
			// ゲームの更新
			game.Update();

			// 終了リクエストが来たら抜ける
			if (game.IsEndRequest()) {
				break;
			}

			// 描画
			game.Draw();
		}

		// ゲームの終了
		game.Finalize();
	}
	catch (const std::exception& exception) {
		const std::string message = std::string("Fatal error: ") + exception.what();
		Logger::Log(message + "\n");
		MessageBoxA(nullptr, message.c_str(), "MadeEngine Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}
	catch (...) {
		constexpr char message[] = "Fatal error: unknown exception.";
		Logger::Log(std::string(message) + "\n");
		MessageBoxA(nullptr, message, "MadeEngine Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
