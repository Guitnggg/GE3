#pragma once

class FrameRateController;
class Time;

/// <summary>フレーム統計とCPUプロファイル結果をImGuiへ表示するデバッグ画面。</summary>
class DebugOverlay final {
  public:
	void Initialize(FrameRateController *frameRateController, Time *time);
	void Draw();
	void Finalize();

	void SetVisible(bool visible) {
		visible_ = visible;
	}
	[[nodiscard]] bool IsVisible() const {
		return visible_;
	}

  private:
	FrameRateController *frameRateController_ = nullptr;
	Time *time_ = nullptr;
	bool visible_ = true;
};
