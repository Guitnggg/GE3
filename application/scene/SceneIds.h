#pragma once

#include <string_view>

namespace GameApp::SceneIds {

// シーン識別子はFactoryとの境界でのみ使用し、具象クラス名をEngine側へ漏らさない。
inline constexpr std::string_view Title = "TITLE";       // タイトル画面
inline constexpr std::string_view Game = "GAME";         // ゲーム本編
inline constexpr std::string_view Clear = "CLEAR";       // ゲームクリア画面
inline constexpr std::string_view GameOver = "GAMEOVER"; // ゲームオーバー画面

} // namespace GameApp::SceneIds
