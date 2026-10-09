#pragma once

class FrameRateController;
class Time;

/// <summary>フレーム統計とCPUプロファイル結果をImGuiへ表示するデバッグ画面。</summary>
class DebugOverlay final {
  public:
	void Initialize(FrameRateController *frameRateController, Time *time);

	/// <summary>現在の診断情報をImGuiウィンドウへ描画する。</summary>
	void Draw();

	/// <summary>参照中のエンジン機能を解放して未初期化状態へ戻す。</summary>
	void Finalize();

	/// <summary>診断ウィンドウの表示状態を設定する。</summary>
	void SetVisible(bool visible) {
		visible_ = visible;
	}

	/// <summary>診断ウィンドウが表示対象かを返す。</summary>
	[[nodiscard]] bool IsVisible() const {
		return visible_;
	}

  private:
	FrameRateController *frameRateController_ = nullptr; // FPSとフレーム時間の参照元
	Time *time_ = nullptr;                               // ゲーム時間と固定更新情報の参照元
	bool visible_ = true;                                // 診断ウィンドウを表示するか
};
