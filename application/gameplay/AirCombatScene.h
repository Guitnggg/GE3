#pragma once

#include "application/gameplay/CombatCamera.h"
#include "application/gameplay/CombatHud.h"
#include "application/gameplay/Fighter.h"
#include "application/gameplay/MissileSystem.h"
#include "engine/scene/Scene.h"

#include <memory>

class Model;

class AirCombatScene final : public Scene {
  protected:
	void OnInitialize() override;
	void OnUpdate() override;
	void OnDraw() override;

  private:
	void ResetBattle();
	void UpdateVisuals();

	std::shared_ptr<Model> fighterModel_;
	Fighter player_;
	Fighter enemy_;
	MissileSystem missiles_;
	CombatCamera camera_;
	CombatHud hud_;
	bool lockedOn_ = false;
	bool battleEnded_ = false;
};
