#pragma once

#include "engine/3D/object/Object3d.h"
#include <memory>

class Camera;
class Model;
class Object3dCommon;
class TextureManager;

/// <summary>
/// 1体の敵の表示、通過判定、射線判定を管理する。
/// </summary>
class Enemy final {
public:
	/// <summary>
	/// 共有球モデルを使用して敵の3Dオブジェクトを生成する。
	/// </summary>
	/// <param name="object3dCommon">3Dオブジェクトの共通描画機能</param>
	/// <param name="textureManager">テクスチャ管理機能</param>
	/// <param name="model">全敵で共有する球モデル</param>
	/// <param name="position">敵を配置するワールド座標</param>
	/// <param name="radius">表示スケールと当たり判定に使用する半径</param>
	void Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
		const std::shared_ptr<Model>& model, const Vector3& position, float radius);

	/// <summary>
	/// 回転を進め、現在のカメラに対する描画行列を更新する。
	/// </summary>
	void Update(const Camera& camera, float deltaTime, float rotationSpeed);

	/// <summary>
	/// 敵の3Dモデルを描画する。
	/// </summary>
	void Draw() const;

	/// <summary>
	/// 敵がカメラ位置を通過したか判定する。
	/// </summary>
	bool IsPassed(float cameraZ) const;

	/// <summary>
	/// 射線と敵の球形当たり判定が交差するか調べる。
	/// </summary>
	/// <param name="origin">射線の始点</param>
	/// <param name="direction">正規化済みの射線方向</param>
	/// <param name="distance">命中時に始点から交点までの距離を受け取る</param>
	/// <returns>射線が敵へ命中した場合はtrue</returns>
	bool IntersectsRay(const Vector3& origin, const Vector3& direction, float& distance) const;

private:
	std::unique_ptr<Object3d> object_; // 敵の表示とワールド座標を所有する3Dオブジェクト
	float radius_ = 1.0f;             // 射線判定に使用する球の半径
};
