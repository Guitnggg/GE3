#include "application/gameplay/CombatHud.h"

#include "application/gameplay/AirCombatMath.h"
#include "application/gameplay/Fighter.h"
#include "engine/assets/AssetManager.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cmath>

using namespace AirCombatMath;

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
	aimMarker_.GetTransform().translate = {kScreenWidth * 0.5f, kScreenHeight * 0.5f, 0.0f};
}

bool CombatHud::UpdateLock(const Fighter &player, const Fighter &enemy, const Camera &camera, double elapsedTime) {
	if (!enemy.IsAlive()) return false;
	const Vector3 toEnemy = Subtract(enemy.GetPosition(), player.GetPosition());
	Vector2 screen{};
	const bool locked = Length(toEnemy) < 650.0f && Dot(player.GetForward(), Normalize(toEnemy)) > 0.55f &&
	                    WorldToScreen(enemy.GetPosition(), camera, screen);
	if (locked) {
		lockMarker_.GetTransform().translate = {screen.x, screen.y, 0.0f};
		const float pulse = 0.85f + std::sin(static_cast<float>(elapsedTime) * 8.0f) * 0.12f;
		lockMarker_.GetTransform().scale = {pulse, pulse, 1.0f};
	}
	return locked;
}

void CombatHud::Draw(const Fighter &player, const Fighter &enemy, bool lockedOn, bool battleEnded) {
	aimMarker_.Update(kScreenWidth, kScreenHeight);
	aimMarker_.Draw();
	if (lockedOn && enemy.IsAlive()) {
		lockMarker_.Update(kScreenWidth, kScreenHeight);
		lockMarker_.Draw();
	}
#ifdef _DEBUG
	ImGui::SetNextWindowPos({18.0f, 18.0f}, ImGuiCond_Once);
	ImGui::SetNextWindowBgAlpha(0.72f);
	if (ImGui::Begin("AIR COMBAT PROTOTYPE")) {
		ImGui::Text("W/S: pitch   A/D: yaw   Q/E: roll");
		ImGui::Text("Shift/Ctrl: throttle   Space: missile");
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
