#include "application/core/MyGame.h"

#include "application/scene/AirCombatScene.h"
#include "engine/debug/Profiler.h"

#include <memory>

//===============
// ゲームライフサイクル
//===============

// 起動時に空戦シーンを生成して、最初に実行するシーンとして登録する
void MyGame::OnInitialize() {
	SetInitialScene(std::make_unique<AirCombatScene>());
}

// ゲーム全体に固有の毎フレーム処理を計測する
void MyGame::OnUpdate() {
	PROFILE_SCOPE("Game Update");
}

// ゲーム全体に固有の固定間隔処理を計測する
void MyGame::OnFixedUpdate() {
	PROFILE_SCOPE("Game FixedUpdate");
}

// ゲーム全体に固有の描画処理を計測する
void MyGame::OnDraw() {
	PROFILE_SCOPE("Game Draw");
}
