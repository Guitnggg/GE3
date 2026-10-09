#pragma once

#include "engine/core/utility/JsonUtility.h"

#include <filesystem>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

/// <summary>ドット区切りの階層キーでJSON設定を安全に読み書きする汎用ストア。</summary>
class SettingsStore final {
  public:
	explicit SettingsStore(uint32_t schemaVersion = 1);

	bool Load(const std::filesystem::path &path, std::string *errorMessage = nullptr);
	bool Save(const std::filesystem::path &path, std::string *errorMessage = nullptr) const;
	void Reset();

	template <typename T>
	[[nodiscard]] T Get(std::string_view key, const T &defaultValue) const {
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

	template <typename T>
	void Set(std::string_view key, T &&value) {
		FindOrCreate(key) = std::forward<T>(value);
	}

	[[nodiscard]] bool Contains(std::string_view key) const;
	bool Remove(std::string_view key);
	[[nodiscard]] uint32_t GetSchemaVersion() const {
		return schemaVersion_;
	}

  private:
	const JsonUtility::Json *Find(std::string_view key) const;
	JsonUtility::Json &FindOrCreate(std::string_view key);

	JsonUtility::Json root_;
	uint32_t schemaVersion_ = 1;
};
