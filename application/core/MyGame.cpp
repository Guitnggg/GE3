#include "application/core/MyGame.h"

#include "engine/debug/Profiler.h"
#include "engine/scene/Scene.h"

namespace {
/// <summary>
/// 描画要素を持たない最小シーン。今後のサンプルゲーム実装の起点として使用する。
/// </summary>
class InitialScene final : public Scene {};
} // namespace

MyGame::MyGame() = default;

void MyGame::OnInitialize() {
	SetInitialScene(std::make_unique<InitialScene>());
}
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
