#pragma once

#include <memory>
#include <string_view>

class IScene;

/// <summary>
/// ゲーム固有の識別子からシーンを生成するための抽象インターフェース。
/// </summary>
class AbstractSceneFactory {
  public:
	virtual ~AbstractSceneFactory() = default;

	/// <summary>
	/// 指定された識別子に対応するシーンを生成する。
	/// </summary>
	virtual std::unique_ptr<IScene> CreateScene(std::string_view sceneId) = 0;
};
