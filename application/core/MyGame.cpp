#include "application/core/MyGame.h"

#include "application/gameplay/AirCombatScene.h"
#include "engine/debug/Profiler.h"
#include <memory>

void MyGame::OnInitialize() { SetInitialScene(std::make_unique<AirCombatScene>()); }
void MyGame::OnUpdate() { PROFILE_SCOPE("Game Update"); }
void MyGame::OnFixedUpdate() { PROFILE_SCOPE("Game FixedUpdate"); }
void MyGame::OnDraw() { PROFILE_SCOPE("Game Draw"); }
