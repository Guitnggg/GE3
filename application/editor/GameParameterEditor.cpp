#include "application/editor/GameParameterEditor.h"

#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

bool GameParameterEditor::Draw(GameParameters& parameters) {
#ifdef _DEBUG
	bool restartRequested = false;
	ImGui::SetNextWindowSize({370.0f, 0.0f}, ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Game Parameter Editor")) {
		if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Rail Speed", &parameters.railSpeed, 0.0f, 30.0f, "%.1f");
			ImGui::SliderFloat("Aim Speed", &parameters.aimSpeed, 100.0f, 1200.0f, "%.0f");
			int lives = static_cast<int>(parameters.startingLives);
			if (ImGui::SliderInt("Starting Lives", &lives, 1, 10)) {
				parameters.startingLives = static_cast<uint32_t>(lives);
			}
		}
		if (ImGui::CollapsingHeader("Enemy", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Spawn Interval", &parameters.enemySpawnInterval, 0.15f, 3.0f, "%.2f s");
			ImGui::SliderFloat("Minimum Interval", &parameters.minimumSpawnInterval, 0.1f, 1.5f, "%.2f s");
			ImGui::SliderFloat("Score Acceleration", &parameters.spawnAccelerationPerScore, 0.0f, 0.05f, "%.3f");
			ImGui::SliderFloat("Spawn Distance", &parameters.enemySpawnDistance, 15.0f, 100.0f, "%.1f");
			ImGui::SliderFloat("Base Radius", &parameters.enemyBaseRadius, 0.2f, 3.0f, "%.2f");
			ImGui::SliderFloat("Radius Variation", &parameters.enemyRadiusStep, 0.0f, 0.8f, "%.2f");
			ImGui::SliderFloat("Rotation Speed", &parameters.enemyRotationSpeed, 0.0f, 8.0f, "%.2f rad/s");
		}
		ImGui::Separator();
		if (ImGui::Button(paused_ ? "Resume Game" : "Pause Game")) {
			paused_ = !paused_;
		}
		ImGui::SameLine();
		if (ImGui::Button("Restart Game")) { restartRequested = true; }
		ImGui::SameLine();
		if (ImGui::Button("Restore Defaults")) {
			parameters = GameParameters{};
			restartRequested = true;
		}
		if (paused_) {
			ImGui::TextColored({1.0f, 0.8f, 0.2f, 1.0f}, "PAUSED");
		}
		ImGui::TextDisabled("Values are applied immediately unless restart is noted.");
	}
	ImGui::End();
	return restartRequested;
#else
	(void)parameters;
	return false;
#endif
}
