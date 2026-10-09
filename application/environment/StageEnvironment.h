#pragma once

#include <memory>
#include <vector>

class Camera;
class Model;
class Object3d;
class Object3dCommon;
class TextureManager;

/// <summary>
/// 街区モデルの生成、循環、描画を管理する。
/// </summary>
class StageEnvironment final {
  public:
	/// <summary>
	/// 所有しているステージオブジェクトを破棄する。
	/// </summary>
	~StageEnvironment();

	/// <summary>
	/// 描画サービスと共有モデルを使って、循環表示する街区を生成する。
	/// </summary>
	void Initialize(Object3dCommon *object3dCommon,
	                TextureManager *textureManager,
	                const std::shared_ptr<Model> &mapModel);
	/// <summary>
	/// 全街区をゲーム開始時の配置へ戻す。
	/// </summary>
	void Reset();
	/// <summary>
	/// カメラ後方へ抜けた街区を前方へ移動し、描画状態を更新する。
	/// </summary>
	void Update(const Camera &camera, float cameraZ);
	/// <summary>
	/// 現在の全街区を描画する。
	/// </summary>
	void Draw() const;
	/// <summary>
	/// 保持している街区と共有モデルへの参照を解放する。
	/// </summary>
	void Finalize();

  private:
	std::shared_ptr<Model> mapModel_;
	std::vector<std::unique_ptr<Object3d>> segments_;
};
