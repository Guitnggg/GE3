#include <Windows.h>

#include "application/MyGame.h"

#pragma comment(lib, "dxcompiler.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
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

	return 0;
}
