#include "application/core/MyGame.h"

#include "application/scene/SceneFactory.h"
#include "application/scene/SceneIds.h"
#include "engine/debug/Profiler.h"
#include "engine/effects/post/PostEffectManager.h"
#include "engine/effects/post/VignettingEffect.h"

namespace GameApp {

void MyGame::OnInitialize() {
	// ゲームで使用するEffectを登録順に追加する。
	// PostEffectManagerは登録されたEffectの所有権を受け取り、同じ順番でPing-Pong実行する。
	postEffectManager_->AddEffect(std::make_unique<VignettingEffect>());

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

void MyGame::OnDrawWorld() {
	PROFILE_SCOPE("Game Draw World");
}

void MyGame::OnDrawUI() {
	PROFILE_SCOPE("Game Draw UI");
}

} // namespace GameApp
