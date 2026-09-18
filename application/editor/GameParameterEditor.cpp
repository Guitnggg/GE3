#include "application/editor/GameParameterEditor.h"

#ifdef _DEBUG
#include "engine/audio/Audio.h"
#include "engine/core/timing/FrameRateController.h"
#include "engine/core/timing/Time.h"
#include "externals/imgui/imgui.h"
#include <stdexcept>
#include <string>
#endif

void GameParameterEditor::Initialize(Audio* audio) {
#ifdef _DEBUG
	if (audio_ != nullptr) { throw std::logic_error("GameParameterEditor is already initialized."); }
	if (audio == nullptr) { throw std::invalid_argument("GameParameterEditor requires Audio."); }
	audio_ = audio;
	soundHandle_ = audio_->Load("fanfare.wav");
#else
	(void)audio;
#endif
}

void GameParameterEditor::Finalize() {
#ifdef _DEBUG
	if (audio_ != nullptr) {
		if (voiceHandle_ != Audio::kInvalidVoiceHandle) { audio_->Stop(voiceHandle_); }
		if (soundHandle_ != Audio::kInvalidSoundHandle) { audio_->Unload(soundHandle_); }
	}
#endif
	audio_ = nullptr;
	soundHandle_ = 0;
	voiceHandle_ = 0;
	audioPaused_ = false;
}

bool GameParameterEditor::Draw(
	GameParameters& parameters, FrameRateController& frameRateController, Time& time) {
#ifdef _DEBUG
	// エディタ操作の結果だけを返し、ゲームの再初期化自体はGameSceneへ任せる
	bool restartRequested = false;
	ImGui::SetNextWindowSize({370.0f, 0.0f}, ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Game Parameter Editor")) {
		// プレイヤー関連の値。初期ライフは次回リスタート時に反映される
		if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Rail Speed", &parameters.railSpeed, 0.0f, 30.0f, "%.1f");
			ImGui::SliderFloat("Move Speed", &parameters.playerMoveSpeed, 1.0f, 15.0f, "%.1f");
			int lives = static_cast<int>(parameters.startingLives);
			if (ImGui::SliderInt("Starting Lives", &lives, 1, 10)) {
				parameters.startingLives = static_cast<uint32_t>(lives);
			}
		}
		// 敵生成と難易度上昇に関する値は変更後の更新から即時反映される
		if (ImGui::CollapsingHeader("Enemy", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Spawn Interval", &parameters.enemySpawnInterval, 0.15f, 3.0f, "%.2f s");
			ImGui::SliderFloat("Minimum Interval", &parameters.minimumSpawnInterval, 0.1f, 1.5f, "%.2f s");
			ImGui::SliderFloat("Score Acceleration", &parameters.spawnAccelerationPerScore, 0.0f, 0.05f, "%.3f");
			ImGui::SliderFloat("Spawn Distance", &parameters.enemySpawnDistance, 15.0f, 100.0f, "%.1f");
			ImGui::SliderFloat("Base Radius", &parameters.enemyBaseRadius, 0.2f, 3.0f, "%.2f");
			ImGui::SliderFloat("Radius Variation", &parameters.enemyRadiusStep, 0.0f, 0.8f, "%.2f");
			ImGui::SliderFloat("Rotation Speed", &parameters.enemyRotationSpeed, 0.0f, 8.0f, "%.2f rad/s");
		}
		DrawAudioControls();
		DrawFrameRateControls(frameRateController);
		DrawTimeControls(time);
		ImGui::Separator();
		// 一時停止は描画とエディタ操作を維持したままゲームロジックだけを止める
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
	// ReleaseビルドではImGuiを使用せず、再スタート要求も発生させない
	(void)parameters;
	(void)frameRateController;
	(void)time;
	return false;
#endif
}

void GameParameterEditor::DrawAudioControls() {
#ifdef _DEBUG
	if (audio_ == nullptr) { return; }
	if (voiceHandle_ != Audio::kInvalidVoiceHandle && !audioPaused_ && !audio_->IsPlaying(voiceHandle_)) {
		voiceHandle_ = Audio::kInvalidVoiceHandle;
	}
	if (!ImGui::CollapsingHeader("Audio")) { return; }

	ImGui::TextUnformatted("resource/audio/fanfare.wav");
	ImGui::Checkbox("Loop", &audioLoop_);
	if (ImGui::SliderFloat("Volume", &audioVolume_, 0.0f, 1.0f, "%.2f") &&
		voiceHandle_ != Audio::kInvalidVoiceHandle) {
		audio_->SetVolume(voiceHandle_, audioVolume_);
	}
	if (ImGui::SliderFloat("Pitch", &audioPitch_, 0.25f, 4.0f, "%.2fx", ImGuiSliderFlags_Logarithmic) &&
		voiceHandle_ != Audio::kInvalidVoiceHandle) {
		audio_->SetPitch(voiceHandle_, audioPitch_);
	}
	if (ImGui::SliderFloat("Master Volume", &masterVolume_, 0.0f, 1.0f, "%.2f")) {
		audio_->SetMasterVolume(masterVolume_);
	}

	if (ImGui::Button("Play / Restart")) {
		if (voiceHandle_ != Audio::kInvalidVoiceHandle) { audio_->Stop(voiceHandle_); }
		voiceHandle_ = audio_->Play(soundHandle_, audioLoop_, audioVolume_, audioPitch_);
		audioPaused_ = false;
	}
	ImGui::SameLine();
	if (!audioPaused_) {
		if (ImGui::Button("Pause") && voiceHandle_ != Audio::kInvalidVoiceHandle) {
			audio_->Pause(voiceHandle_);
			audioPaused_ = true;
		}
	} else if (ImGui::Button("Resume") && voiceHandle_ != Audio::kInvalidVoiceHandle) {
		audio_->Resume(voiceHandle_);
		audioPaused_ = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop") && voiceHandle_ != Audio::kInvalidVoiceHandle) {
		audio_->Stop(voiceHandle_);
		voiceHandle_ = Audio::kInvalidVoiceHandle;
		audioPaused_ = false;
	}

	const char* status = "Stopped";
	if (voiceHandle_ != Audio::kInvalidVoiceHandle) { status = audioPaused_ ? "Paused" : "Playing"; }
	ImGui::Text("Status: %s", status);
#endif
}

void GameParameterEditor::DrawFrameRateControls(FrameRateController& frameRateController) {
#ifdef _DEBUG
	if (!ImGui::CollapsingHeader("Frame Rate")) { return; }
	static constexpr const char* kModeNames[] = {"VSync", "Limited", "Unlimited"};
	int modeIndex = static_cast<int>(frameRateController.GetMode());
	if (ImGui::Combo("Mode", &modeIndex, kModeNames, IM_ARRAYSIZE(kModeNames))) {
		frameRateController.SetMode(static_cast<FrameRateMode>(modeIndex));
	}
	float targetFPS = static_cast<float>(frameRateController.GetTargetFPS());
	if (ImGui::SliderFloat("Target FPS", &targetFPS, 15.0f, 500.0f, "%.0f FPS")) {
		frameRateController.SetTargetFPS(targetFPS);
	}
	static constexpr double kPresets[] = {30.0, 60.0, 120.0, 144.0, 165.0, 240.0};
	for (int index = 0; index < IM_ARRAYSIZE(kPresets); ++index) {
		if (index > 0) { ImGui::SameLine(); }
		const std::string label = std::to_string(static_cast<int>(kPresets[index]));
		if (ImGui::SmallButton(label.c_str())) { frameRateController.SetTargetFPS(kPresets[index]); }
	}
	ImGui::Text("Current: %.1f FPS (%.2f ms)",
		frameRateController.GetCurrentFPS(), frameRateController.GetFrameTimeMilliseconds());
	if (frameRateController.GetMode() == FrameRateMode::VSync) {
		ImGui::TextUnformatted("Target FPS is ignored while VSync is enabled.");
	}
#else
	(void)frameRateController;
#endif
}

void GameParameterEditor::DrawTimeControls(Time& time) {
#ifdef _DEBUG
	if (!ImGui::CollapsingHeader("Fixed Update")) { return; }
	float fixedUpdateRate = 1.0f / time.GetFixedDeltaTime();
	if (ImGui::SliderFloat("Fixed Update Rate", &fixedUpdateRate, 15.0f, 240.0f, "%.0f Hz")) {
		time.SetFixedDeltaTime(1.0f / fixedUpdateRate);
	}
	int maxSteps = static_cast<int>(time.GetMaxFixedStepsPerFrame());
	if (ImGui::SliderInt("Max Catch-up Steps", &maxSteps, 1, 16)) {
		time.SetMaxFixedStepsPerFrame(static_cast<uint32_t>(maxSteps));
	}
	ImGui::Text("Previous frame steps: %u", time.GetFixedStepsThisFrame());
	ImGui::Text("Interpolation alpha: %.3f", time.GetFixedInterpolationAlpha());
	if (time.WasFixedTimeDroppedThisFrame()) {
		ImGui::TextColored({1.0f, 0.65f, 0.2f, 1.0f},
			"Catch-up limit reached; excess fixed time was dropped.");
	}
#else
	(void)time;
#endif
}
