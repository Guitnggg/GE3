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
	~StageEnvironment();
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& mapModel);
	void Reset();
	void Update(const Camera& camera, float cameraZ);
	void Draw() const;
	void Finalize();

private:
	std::shared_ptr<Model> mapModel_;
	std::vector<std::unique_ptr<Object3d>> segments_;
};
