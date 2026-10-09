#include "engine/debug/DebugOverlay.h"

#include "engine/core/timing/FrameRateController.h"
#include "engine/core/timing/Time.h"
#include "engine/debug/Profiler.h"
#include "externals/imgui/imgui.h"

#include <stdexcept>

void DebugOverlay::Initialize(FrameRateController *frameRateController, Time *time) {
	if (frameRateController == nullptr || time == nullptr) {
		throw std::invalid_argument("DebugOverlay requires FrameRateController and Time.");
	}
	frameRateController_ = frameRateController;
	time_ = time;
}

void DebugOverlay::Draw() {
	if (!visible_) {
		return;
	}
	if (frameRateController_ == nullptr || time_ == nullptr) {
		throw std::logic_error("DebugOverlay is not initialized.");
	}

	ImGui::SetNextWindowBgAlpha(0.85f);
	if (ImGui::Begin("Engine Diagnostics", &visible_, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("FPS: %.1f", frameRateController_->GetCurrentFPS());
		ImGui::Text("Frame: %.3f ms", frameRateController_->GetFrameTimeMilliseconds());
		ImGui::Text("Delta: %.3f ms", static_cast<double>(time_->GetUnscaledDeltaTime()) * 1000.0);
		ImGui::Text("Frame count: %llu", static_cast<unsigned long long>(time_->GetFrameCount()));
		ImGui::Text("Fixed steps: %u / %u", time_->GetFixedStepsThisFrame(), time_->GetMaxFixedStepsPerFrame());
		if (time_->WasFixedTimeDroppedThisFrame()) {
			ImGui::TextColored({1.0f, 0.3f, 0.2f, 1.0f}, "Fixed update time was dropped");
		}

		if (ImGui::CollapsingHeader("CPU Profiler", ImGuiTreeNodeFlags_DefaultOpen)) {
			const Profiler &profiler = Profiler::Get();
			ImGui::Text("Profiled total: %.3f ms", profiler.GetProfiledMilliseconds());
			ImGui::Separator();
			for (const ProfileRecord &record : profiler.GetLastFrameRecords()) {
				ImGui::Text("%-24s %8.3f ms  x%u", record.name.c_str(), record.milliseconds, record.callCount);
			}
		}
	}
	ImGui::End();
}

void DebugOverlay::Finalize() {
	frameRateController_ = nullptr;
	time_ = nullptr;
}
