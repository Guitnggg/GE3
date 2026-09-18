#pragma once

#include "externals/nlohmann/json.hpp"

#include <filesystem>
#include <string>

/// <summary>
/// JSONファイルの読み書きを行うユーティリティ関数群。
/// </summary>
namespace JsonUtility
{
	using Json = nlohmann::json;

	/// <summary>
	/// JSONファイルを読み込む。
	/// </summary>
	bool Load(const std::filesystem::path& path, Json& json, std::string* errorMessage = nullptr);

	/// <summary>
	/// JSONをファイルへ保存する。保存先の親フォルダがなければ作成する。
	/// </summary>
	bool Save(const std::filesystem::path& path, const Json& json, int indent = 4, std::string* errorMessage = nullptr);
}
