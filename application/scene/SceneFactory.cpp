#include "application/scene/SceneFactory.h"

#include "application/scene/GameClearScene.h"
#include "application/scene/GameOverScene.h"
#include "application/scene/GameScene.h"
#include "application/scene/SceneIds.h"
#include "application/scene/TitleScene.h"

#include <stdexcept>
#include <string>

namespace GameApp {

std::unique_ptr<IScene> SceneFactory::CreateScene(std::string_view sceneId) {
	// ゲーム固有のIDと具象クラスの対応は、このFactoryだけに集約する。
	if (sceneId == SceneIds::Title) {
		return std::make_unique<TitleScene>();
	}

	if (sceneId == SceneIds::Game) {
		return std::make_unique<GameScene>();
	}

	if (sceneId == SceneIds::Clear) {
		return std::make_unique<GameClearScene>();
	}

	if (sceneId == SceneIds::GameOver) {
		return std::make_unique<GameOverScene>();
	}

	// IDのタイプミスを空のシーンとして扱わず、生成時点で明確に失敗させる。
	throw std::invalid_argument("Unknown scene identifier: " + std::string(sceneId));
}

} // namespace GameApp
