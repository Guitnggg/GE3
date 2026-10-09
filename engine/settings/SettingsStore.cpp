#include "engine/settings/SettingsStore.h"

#include <stdexcept>
#include <tuple>

namespace {
std::pair<std::string_view, std::string_view> Split(std::string_view key) {
	const size_t separator = key.find('.');
	return separator == std::string_view::npos ? std::pair{key, std::string_view{}}
	                                           : std::pair{key.substr(0, separator), key.substr(separator + 1)};
}
} // namespace

SettingsStore::SettingsStore(uint32_t schemaVersion) : schemaVersion_(schemaVersion) {
	if (schemaVersion == 0) {
		throw std::invalid_argument("Settings schema version must be greater than zero.");
	}
	Reset();
}

bool SettingsStore::Load(const std::filesystem::path &path, std::string *errorMessage) {
	JsonUtility::Json loaded;
	if (!JsonUtility::Load(path, loaded, errorMessage)) {
		return false;
	}
	if (!loaded.is_object() || loaded.value("schemaVersion", 0u) != schemaVersion_) {
		if (errorMessage) {
			*errorMessage = "Settings schema version is unsupported.";
		}
		return false;
	}
	root_ = std::move(loaded);
	return true;
}

bool SettingsStore::Save(const std::filesystem::path &path, std::string *errorMessage) const {
	return JsonUtility::Save(path, root_, 4, errorMessage);
}

void SettingsStore::Reset() {
	root_ = JsonUtility::Json::object();
	root_["schemaVersion"] = schemaVersion_;
}

bool SettingsStore::Contains(std::string_view key) const {
	return Find(key) != nullptr;
}

bool SettingsStore::Remove(std::string_view key) {
	auto [head, tail] = Split(key);
	if (head.empty()) {
		return false;
	}
	JsonUtility::Json *node = &root_;
	while (!tail.empty()) {
		if (!node->is_object() || !node->contains(std::string(head))) {
			return false;
		}
		node = &(*node)[std::string(head)];
		std::tie(head, tail) = Split(tail);
	}
	return node->is_object() && node->erase(std::string(head)) != 0;
}

const JsonUtility::Json *SettingsStore::Find(std::string_view key) const {
	if (key.empty()) {
		return nullptr;
	}
	const JsonUtility::Json *node = &root_;
	while (!key.empty()) {
		auto [head, tail] = Split(key);
		if (head.empty() || !node->is_object()) {
			return nullptr;
		}
		const auto found = node->find(std::string(head));
		if (found == node->end()) {
			return nullptr;
		}
		node = &*found;
		key = tail;
	}
	return node;
}

JsonUtility::Json &SettingsStore::FindOrCreate(std::string_view key) {
	if (key.empty()) {
		throw std::invalid_argument("Settings key must not be empty.");
	}
	JsonUtility::Json *node = &root_;
	while (true) {
		auto [head, tail] = Split(key);
		if (head.empty()) {
			throw std::invalid_argument("Settings key contains an empty segment.");
		}
		if (tail.empty()) {
			return (*node)[std::string(head)];
		}
		JsonUtility::Json &child = (*node)[std::string(head)];
		if (!child.is_object()) {
			child = JsonUtility::Json::object();
		}
		node = &child;
		key = tail;
	}
}
