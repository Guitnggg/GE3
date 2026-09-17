#pragma once

#include "engine/2D/Sprite.h"
#include "engine/3D/camera/Camera.h"
#include "engine/math/Mymath.h"
#include <cstdint>
#include <memory>
#include <vector>

class Input;
class SpriteCommon;
class TextureManager;

/// <summary>
/// レール上を進むプレイヤーのカメラ、照準、ライフを管理する。
/// </summary>
class Player final {
public:
	/// <summary>入力、描画共通機能、照準用テクスチャを受け取って初期化する。</summary>
	/// <param name="spriteCommon">2Dスプライトの共通描画機能</param>
	/// <param name="textureManager">テクスチャ管理機能</param>
	/// <param name="input">キーボード入力機能</param>
	/// <param name="texture">照準に使用するテクスチャ番号</param>
	void Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, Input* input, uint32_t texture);

	/// <summary>カメラ、照準、ライフをゲーム開始時の状態へ戻す。</summary>
	/// <param name="startingLives">ゲーム開始時のライフ数</param>
	void Reset(uint32_t startingLives = 3);

	/// <summary>入力に応じて照準を動かし、カメラをレール方向へ進める。</summary>
	/// <param name="deltaTime">前フレームからの経過秒数</param>
	/// <param name="railSpeed">カメラの前進速度</param>
	/// <param name="acceptFireInput">falseの場合は射撃だけを無効化する</param>
	/// <returns>このフレームで射撃入力された場合はtrue</returns>
	bool Update(float deltaTime, float railSpeed, bool acceptFireInput = true);

	/// <summary>2本のスプライトで構成した照準を描画する。</summary>
	void DrawReticle() const;

	/// <summary>ライフを1減らす。0未満にはしない。</summary>
	void Damage();

	/// <summary>敵などの3Dオブジェクト更新に使用するカメラを取得する。</summary>
	const Camera& GetCamera() const { return camera_; }
	/// <summary>レール上における現在のカメラZ座標を取得する。</summary>
	float GetCameraZ() const { return cameraZ_; }
	/// <summary>現在の残りライフ数を取得する。</summary>
	uint32_t GetLives() const { return lives_; }
	/// <summary>ライフが尽きているか取得する。</summary>
	bool IsDead() const { return lives_ == 0; }
	/// <summary>射線の始点となるカメラ位置を取得する。</summary>
	Vector3 GetShotOrigin() const;
	/// <summary>画面上の照準位置からワールド空間の射線方向を計算する。</summary>
	Vector3 GetShotDirection() const;

private:
	/// <summary>照準を構成する矩形スプライトを1つ生成する。</summary>
	std::unique_ptr<Sprite> CreateReticlePart(float width, float height, uint32_t texture);
	/// <summary>現在の照準位置と射撃演出を各スプライトへ反映する。</summary>
	void UpdateReticle();

	Input* input_ = nullptr;                         // Frameworkが所有する入力機能への非所有参照
	SpriteCommon* spriteCommon_ = nullptr;           // 照準描画に使用する共通機能への非所有参照
	TextureManager* textureManager_ = nullptr;       // テクスチャ管理機能への非所有参照
	Camera camera_{};                                // プレイヤー視点の3Dカメラ
	std::vector<std::unique_ptr<Sprite>> reticle_;   // 照準を構成するスプライト
	Vector2 aim_{640.0f, 360.0f};                    // ピクセル単位の照準座標
	float cameraZ_ = -10.5f;                         // レール上の現在位置
	float shotFlashTimer_ = 0.0f;                    // 射撃時に照準色を変える残り時間
	uint32_t lives_ = 3;                             // 敵を逃せる残り回数
};
