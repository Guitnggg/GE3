#include "application/scene/TitleScene.h"

#include "application/scene/SceneIds.h"
#include "engine/input/Input.h"
#include "engine/scene/SceneManager.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

namespace GameApp {

void TitleScene::OnUpdate() {
	// 切り替えは即時実行されず、SceneManagerが次のフレーム境界で安全に適用する。
	if (GetContext().input->TriggerKey(DIK_RETURN)) {
		GetContext().sceneManager->ChangeScene(SceneIds::Game);
	}
}

void TitleScene::OnDrawUI() {
#ifdef _DEBUG
	// 仮のタイトル表示。Sprite UI実装後は、このフック内で置き換える。
	ImGui::Begin("Scene");
	ImGui::TextUnformatted("TITLE");
	ImGui::TextUnformatted("Press Enter to start");
	ImGui::End();
#endif
}

} // namespace GameApp
