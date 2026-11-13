#include "application/editor/GameEditor.h"

#include "application/actor/Fighter.h"
#include "application/weapon/MissileSystem.h"
#include "engine/core/timing/Time.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"

#include <algorithm>
#include <numbers>

namespace {
constexpr float kRadiansToDegrees = 180.0f / std::numbers::pi_v<float>;
constexpr float kDegreesToRadians = std::numbers::pi_v<float> / 180.0f;
}
#endif

bool GameEditor::Draw(Fighter &player, Fighter &enemy, MissileSystem &missiles, Time &time,
                      bool lockedOn, bool battleEnded) {
#ifndef _DEBUG
	(void)player;
	(void)enemy;
	(void)missiles;
	(void)time;
	(void)lockedOn;
	(void)battleEnded;
	return false;
#else
	bool resetRequested = false;
	const bool paused = time.GetTimeScale() == 0.0f;
	ImGui::SetNextWindowSize({420.0f, 650.0f}, ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Game Editor", nullptr, ImGuiWindowFlags_MenuBar)) {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Game")) {
				if (ImGui::MenuItem("Reset battle", "R")) {
					resetRequested = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::CollapsingHeader("Play Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::Button(paused ? "Resume" : "Pause")) {
				if (paused) {
					time.SetTimeScale(std::max(runningTimeScale_, 0.01f));
				} else {
					runningTimeScale_ = time.GetTimeScale();
					time.SetTimeScale(0.0f);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset Battle")) {
				resetRequested = true;
			}

			float timeScale = paused ? runningTimeScale_ : time.GetTimeScale();
			if (ImGui::SliderFloat("Time Scale", &timeScale, 0.05f, 3.0f, "%.2fx")) {
				runningTimeScale_ = timeScale;
				if (!paused) {
					time.SetTimeScale(timeScale);
				}
			}
			ImGui::Text("Frame: %llu  Game time: %.2f s",
			            static_cast<unsigned long long>(time.GetFrameCount()), time.GetElapsedTime());
		}

		if (ImGui::CollapsingHeader("Battle Monitor", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("State: %s", battleEnded ? "Finished" : (paused ? "Paused" : "Playing"));
			ImGui::TextColored(lockedOn ? ImVec4{1.0f, 0.3f, 0.2f, 1.0f}
			                            : ImVec4{0.65f, 0.65f, 0.65f, 1.0f},
			                   "Lock-on: %s", lockedOn ? "Active" : "None");
			ImGui::Text("Active missiles: %zu", missiles.missiles_.size());
			if (!missiles.missiles_.empty() && ImGui::Button("Clear Missiles")) {
				missiles.Reset();
			}
		}

		DrawFighterEditor("Player", player);
		DrawFighterEditor("Enemy", enemy);

		if (ImGui::CollapsingHeader("Help")) {
			ImGui::TextWrapped("Edit transforms while paused for precise placement. "
			                   "Values are live and are intentionally not saved to disk.");
		}
	}
	ImGui::End();
	return resetRequested;
#endif
}

void GameEditor::DrawFighterEditor(const char *label, Fighter &fighter) {
#ifdef _DEBUG
	if (!ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	ImGui::PushID(label);
	ImGui::DragFloat3("Position", &fighter.position_.x, 0.5f, -4000.0f, 4000.0f, "%.1f");

	float rotation[] = {fighter.pitch_ * kRadiansToDegrees, fighter.yaw_ * kRadiansToDegrees,
	                    fighter.roll_ * kRadiansToDegrees};
	if (ImGui::DragFloat3("Pitch / Yaw / Roll", rotation, 0.5f, -180.0f, 180.0f, "%.1f deg")) {
		fighter.pitch_ = rotation[0] * kDegreesToRadians;
		fighter.yaw_ = rotation[1] * kDegreesToRadians;
		fighter.roll_ = rotation[2] * kDegreesToRadians;
	}
	ImGui::SliderFloat("Speed", &fighter.speed_, 0.0f, 150.0f, "%.1f");
	ImGui::SliderInt("Health", &fighter.health_, 0, 3);
	if (ImGui::Button("Repair")) {
		fighter.health_ = 3;
	}
	ImGui::SameLine();
	if (ImGui::Button("Destroy")) {
		fighter.health_ = 0;
	}
	ImGui::PopID();
#else
	(void)label;
	(void)fighter;
#endif
}
