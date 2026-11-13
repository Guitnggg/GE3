#pragma once

class Fighter;
class MissileSystem;
class Time;

/// <summary>空戦ゲーム全体の状態を実行中に確認・編集するImGuiツール。</summary>
class GameEditor final {
  public:
	/// <summary>エディタを描画し、要求された操作をゲーム状態へ反映する。</summary>
	/// <returns>戦闘リセットが要求された場合はtrue</returns>
	bool Draw(Fighter &player, Fighter &enemy, MissileSystem &missiles, Time &time,
	          bool lockedOn, bool battleEnded);

  private:
	void DrawFighterEditor(const char *label, Fighter &fighter);

	float runningTimeScale_ = 1.0f;
};
