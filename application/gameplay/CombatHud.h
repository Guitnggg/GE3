#pragma once

#include "engine/2D/Sprite.h"

class AssetManager;
class Camera;
class Fighter;
class SpriteCommon;
class TextureManager;

class CombatHud final {
  public:
	void Initialize(AssetManager &assets, SpriteCommon *spriteCommon, TextureManager *textureManager);
	bool UpdateLock(const Fighter &player, const Fighter &enemy, const Camera &camera, double elapsedTime);
	void Draw(const Fighter &player, const Fighter &enemy, bool lockedOn, bool battleEnded);

  private:
	Sprite lockMarker_;
	Sprite aimMarker_;
};
