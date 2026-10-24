#include "application/core/MyGame.h"

#include "application/scene/SceneFactory.h"
#include "application/scene/SceneIds.h"
#include "engine/debug/Profiler.h"

namespace GameApp {

void MyGame::OnInitialize() {
	// エンジンはゲーム固有のクラスを知らないため、生成処理をFactoryとして渡す。
	SetSceneFactory(std::make_unique<SceneFactory>());

	// 登録したFactoryを使い、タイトルシーンからゲームを開始する。
	SetInitialScene(SceneIds::Title);
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

} // namespace GameApp
