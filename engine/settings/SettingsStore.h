#pragma once

#include "engine/core/utility/JsonUtility.h"

#include <filesystem>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

/// <summary>
/// ドット区切りの階層キーでJSON設定を安全に読み書きする汎用ストア。
/// </summary>
class SettingsStore final {
  public:
	explicit SettingsStore(uint32_t schemaVersion = 1);

	/// <summary>
	/// 指定したJSONファイルから設定値を読み込む。
	/// </summary>
	bool Load(const std::filesystem::path &path, std::string *errorMessage = nullptr);

	/// <summary>
	/// 現在の設定値をJSONファイルへ保存する。
	/// </summary>
	bool Save(const std::filesystem::path &path, std::string *errorMessage = nullptr) const;

	/// <summary>
	/// 保持している設定値を初期状態へ戻す。
	/// </summary>
	void Reset();

	/// <summary>
	/// キーに対応する値を取得し、存在しない場合は既定値を返す。
	/// </summary>
	template <typename T> [[nodiscard]] T Get(std::string_view key, const T &defaultValue) const {
		const JsonUtility::Json *value = Find(key);
		if (value == nullptr) {
			return defaultValue;
		}
		try {
			return value->get<T>();
		} catch (const nlohmann::json::exception &) {
			return defaultValue;
		}
	}

	/// <summary>
	/// 指定したキーへ値を設定する。
	/// </summary>
	template <typename T> void Set(std::string_view key, T &&value) {
		FindOrCreate(key) = std::forward<T>(value);
	}

	/// <summary>
	/// 指定したキーが登録されているかを返す。
	/// </summary>
	[[nodiscard]] bool Contains(std::string_view key) const;

	/// <summary>
	/// 指定したキーと値を削除する。
	/// </summary>
	bool Remove(std::string_view key);

	/// <summary>
	/// 設定ファイル形式のバージョンを返す。
	/// </summary>
	[[nodiscard]] uint32_t GetSchemaVersion() const {
		return schemaVersion_;
	}

  private:
	/// <summary>
	/// キーに対応する読み取り専用のJSON値を検索する。
	/// </summary>
	const JsonUtility::Json *Find(std::string_view key) const;

	/// <summary>
	/// キーに対応するJSON値を検索し、必要なら階層を作成する。
	/// </summary>
	JsonUtility::Json &FindOrCreate(std::string_view key);

	JsonUtility::Json root_;     // 読み込み中または編集済みのJSONルート
	uint32_t schemaVersion_ = 1; // 対応する設定ファイル形式のバージョン
};
