#include "application/core/MyGame.h"

#include "engine/debug/Profiler.h"

MyGame::MyGame() = default;

void MyGame::OnInitialize() {}
void MyGame::OnUpdate() {
	PROFILE_SCOPE("Game Update");
}
void MyGame::OnFixedUpdate() {
	PROFILE_SCOPE("Game FixedUpdate");
}
void MyGame::OnDraw() {
	PROFILE_SCOPE("Game Draw");
}
void MyGame::OnFinalize() {}
