#include "tests/TestFramework.h"

#include "engine/settings/SettingsStore.h"

#include <filesystem>
#include <string>

TEST_CASE("SettingsStore reads nested values and defaults") {
	SettingsStore settings;
	settings.Set("video.vsync", false);
	settings.Set("video.width", 1920);
	TestFramework::RequireEqual(settings.Get("video.vsync", true), false, "Nested bool setting is incorrect.");
	TestFramework::RequireEqual(settings.Get("video.width", 0), 1920, "Nested integer setting is incorrect.");
	TestFramework::RequireEqual(settings.Get("missing.value", 60), 60, "Missing setting did not use default.");
}

TEST_CASE("SettingsStore saves and reloads JSON") {
	const std::filesystem::path path = std::filesystem::temp_directory_path() / "made_engine_settings_test.json";
	struct Cleanup {
		std::filesystem::path path;
		~Cleanup() { std::error_code error; std::filesystem::remove(path, error); }
	} cleanup{path};

	SettingsStore source(3);
	source.Set("audio.masterVolume", 0.75f);
	std::string error;
	TestFramework::Require(source.Save(path, &error), "Failed to save settings: " + error);
	SettingsStore loaded(3);
	TestFramework::Require(loaded.Load(path, &error), "Failed to reload settings: " + error);
	TestFramework::RequireNear(loaded.Get("audio.masterVolume", 0.0f), 0.75f, 0.0001f,
	                           "Round-tripped setting is incorrect.");
}

