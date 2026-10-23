#pragma once

#include "engine/2D/Sprite.h"

class AssetManager;
class Camera;
class Fighter;
class SpriteCommon;
class TextureManager;

/// <summary>
/// 照準、ロックマーカー、機体情報を画面へ表示するクラス
/// </summary>
class CombatHud final {
  public:
	/// <summary>
	/// HUDで使用するスプライトを初期化する
	/// </summary>
	void Initialize(AssetManager &assets, SpriteCommon *spriteCommon, TextureManager *textureManager);

	/// <summary>
	/// ロック状態に応じてマーカーの位置と点滅を更新する
	/// </summary>
	void UpdateLockMarker(bool lockedOn, const Vector2 &screenPosition, double elapsedTime);

	/// <summary>
	/// 照準、ロックマーカー、デバッグ用戦闘情報を描画する
	/// </summary>
	void Draw(const Fighter &player, const Fighter &enemy, bool lockedOn, bool battleEnded);

  private:
	Sprite lockMarker_; // 敵機の画面位置へ表示するロックマーカー
	Sprite aimMarker_;  // 画面中央へ固定表示する照準
};
