#include "application/ui/CombatHud.h"

#include "application/actor/Fighter.h"
#include "engine/assets/AssetManager.h"
#include "engine/core/WinApp.h"
#include "engine/math/SpatialMath.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cmath>

using namespace SpatialMath;

namespace {
constexpr float kScreenWidth = static_cast<float>(WinApp::kClientWidth);
constexpr float kScreenHeight = static_cast<float>(WinApp::kClientHeight);
}

//===============
// 初期化
//===============

// 照準とロックマーカーに共通テクスチャを設定する
void CombatHud::Initialize(AssetManager &assets, SpriteCommon *spriteCommon, TextureManager *textureManager) {
	const auto texture = assets.LoadTexture("textures/Lockon.png");
	lockMarker_.Initialize(spriteCommon, textureManager, texture);
	lockMarker_.SetSize({72.0f, 72.0f});
	lockMarker_.SetAnchorPoint({0.5f, 0.5f});
	lockMarker_.SetColor({1.0f, 0.2f, 0.12f, 1.0f});
	aimMarker_.Initialize(spriteCommon, textureManager, texture);
	aimMarker_.SetSize({34.0f, 34.0f});
	aimMarker_.SetAnchorPoint({0.5f, 0.5f});
	aimMarker_.SetColor({0.35f, 0.9f, 1.0f, 0.65f});

	// 通常照準は常に画面中央へ固定する。
	aimMarker_.GetTransform().translate = {kScreenWidth * 0.5f, kScreenHeight * 0.5f, 0.0f};
}

//===============
// 更新・描画
//===============

// ロック中だけ敵機の画面座標へマーカーを移動し、拡縮で点滅させる
void CombatHud::UpdateLockMarker(bool lockedOn, const Vector2 &screenPosition, double elapsedTime) {
	if (lockedOn) {
		lockMarker_.GetTransform().translate = {screenPosition.x, screenPosition.y, 0.0f};
		const float pulse = 0.85f + std::sin(static_cast<float>(elapsedTime) * 8.0f) * 0.12f;
		lockMarker_.GetTransform().scale = {pulse, pulse, 1.0f};
	}
}

// 照準とロックマーカーを描画し、Debugビルドでは戦闘情報も表示する
void CombatHud::Draw(const Fighter &player, const Fighter &enemy, bool lockedOn, bool battleEnded) {
	aimMarker_.Update(kScreenWidth, kScreenHeight);
	aimMarker_.Draw();

	if (lockedOn && enemy.IsAlive()) {
		lockMarker_.Update(kScreenWidth, kScreenHeight);
		lockMarker_.Draw();
	}

#ifdef _DEBUG
	// 右上角を基準に、画面端から少し余白を空けて初期配置する。
	ImGui::SetNextWindowPos({kScreenWidth - 18.0f, 18.0f}, ImGuiCond_Once, {1.0f, 0.0f});
	ImGui::SetNextWindowBgAlpha(0.72f);
	if (ImGui::Begin("AIR COMBAT PROTOTYPE")) {
		ImGui::Text("W/S: pitch   A/D: yaw   Mouse X: roll");
		ImGui::Text("LMB/RMB: accelerate/decelerate   Space: missile");
		ImGui::Separator();
		ImGui::Text("PLAYER  HP %d/3   SPEED %.0f", player.GetHealth(), player.GetSpeed());
		ImGui::Text("ENEMY   HP %d/3   RANGE %.0f", std::max(enemy.GetHealth(), 0), Length(Subtract(enemy.GetPosition(), player.GetPosition())));
		ImGui::TextColored(lockedOn ? ImVec4{1.0f, 0.25f, 0.15f, 1.0f} : ImVec4{0.65f, 0.65f, 0.65f, 1.0f}, lockedOn ? "LOCKED - FIRE!" : "SEARCHING TARGET");
		if (battleEnded) {
			ImGui::Separator();
			ImGui::TextColored(player.IsAlive() ? ImVec4{0.3f, 1.0f, 0.4f, 1.0f} : ImVec4{1.0f, 0.25f, 0.2f, 1.0f}, player.IsAlive() ? "MISSION ACCOMPLISHED" : "MISSION FAILED");
			ImGui::Text("Press R to restart");
		}
	}
	ImGui::End();
#endif
}
