#include "application/scene/GameClearScene.h"

#include "application/scene/SceneIds.h"
#include "engine/input/Input.h"
#include "engine/scene/SceneManager.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

namespace GameApp {

void GameClearScene::OnUpdate() {
	// 結果確認後は、新しいタイトルシーンへ置き換えてゲームを一周させる。
	if (GetContext().input->TriggerKey(DIK_RETURN)) {
		GetContext().sceneManager->ChangeScene(SceneIds::Title);
	}
}

void GameClearScene::OnDrawUI() {
#ifdef _DEBUG
	// 仮の結果表示。Sprite UI実装後も同じUI描画フックを使用できる。
	ImGui::Begin("Scene");
	ImGui::TextUnformatted("CLEAR");
	ImGui::TextUnformatted("Press Enter to return to title");
	ImGui::End();
#endif
}

} // namespace GameApp
