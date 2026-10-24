#pragma once

#include "engine/scene/AbstractSceneFactory.h"

namespace GameApp {

/// <summary>
/// ゲームで使用するシーンIDと、具体的なシーンクラスの対応を管理する。
/// Engine側はこのFactoryを介することで、ゲーム固有クラスへの依存を持たずに済む。
/// </summary>
class SceneFactory final : public AbstractSceneFactory {
  public:
	/// <summary>
	/// シーンIDに対応する新しいシーンを生成し、その所有権を呼び出し元へ渡す。
	/// </summary>
	std::unique_ptr<IScene> CreateScene(std::string_view sceneId) override;
};

} // namespace GameApp
