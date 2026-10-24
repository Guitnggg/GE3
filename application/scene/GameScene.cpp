#include "application/scene/GameScene.h"

#include "application/scene/SceneIds.h"
#include "engine/input/Input.h"
#include "engine/scene/SceneManager.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

namespace GameApp {

void GameScene::OnUpdate() {
	// 現段階では遷移経路を確認するため、デバッグ用キーで結果を選択する。
	if (GetContext().input->TriggerKey(DIK_C)) {
		GetContext().sceneManager->ChangeScene(SceneIds::Clear);
	} else if (GetContext().input->TriggerKey(DIK_G)) {
		GetContext().sceneManager->ChangeScene(SceneIds::GameOver);
	}
}

void GameScene::OnDrawUI() {
#ifdef _DEBUG
	// シーン遷移を確認するための仮UI。ゲーム本編の3D描画とは分離しておく。
	ImGui::Begin("Scene");
	ImGui::TextUnformatted("GAME");
	ImGui::TextUnformatted("C: Clear / G: Game Over");
	ImGui::End();
#endif
}

} // namespace GameApp
