#include "tests/TestFramework.h"

#include "engine/scene/SceneManager.h"

#include <memory>
#include <string>
#include <vector>

namespace {
class RecordingScene final : public IScene {
  public:
	explicit RecordingScene(std::vector<std::string> &events, std::string name)
	    : events_(events), name_(std::move(name)) {}

	void Initialize(const SceneContext &) override { events_.push_back(name_ + ".Initialize"); }
	void OnPause() override { events_.push_back(name_ + ".Pause"); }
	void OnResume() override { events_.push_back(name_ + ".Resume"); }
	void Update() override { events_.push_back(name_ + ".Update"); }
	void FixedUpdate() override { events_.push_back(name_ + ".FixedUpdate"); }
	void Draw() override { events_.push_back(name_ + ".Draw"); }
	void Finalize() override { events_.push_back(name_ + ".Finalize"); }

  private:
	std::vector<std::string> &events_;
	std::string name_;
};
} // namespace

TEST_CASE("SceneManager applies scene changes only at frame boundaries") {
	std::vector<std::string> events;
	SceneManager manager;
	manager.Initialize({}, std::make_unique<RecordingScene>(events, "first"));
	manager.ChangeScene(std::make_unique<RecordingScene>(events, "second"));

	// 予約直後は現在シーンを維持し、BeginFrameで初めて所有権を切り替える。
	manager.Draw();
	TestFramework::RequireEqual(events.back(), std::string("first.Draw"), "Scene changed before BeginFrame.");
	manager.BeginFrame();
	manager.Update();
	TestFramework::RequireEqual(events[2], std::string("first.Finalize"), "Previous scene was not finalized.");
	TestFramework::RequireEqual(events[3], std::string("second.Initialize"), "Next scene was not initialized.");
	manager.Finalize();
}

TEST_CASE("SceneManager layer options control lower scene updates") {
	std::vector<std::string> events;
	SceneManager manager;
	manager.Initialize({}, std::make_unique<RecordingScene>(events, "base"));
	manager.PushScene(std::make_unique<RecordingScene>(events, "overlay"), {false, true});
	manager.BeginFrame();
	events.clear();
	manager.Update();
	manager.Draw();

	TestFramework::RequireEqual(events.size(), size_t{3}, "Unexpected layer callback count.");
	TestFramework::RequireEqual(events[0], std::string("overlay.Update"), "Lower scene should be paused for updates.");
	TestFramework::RequireEqual(events[1], std::string("base.Draw"), "Base scene should draw below overlay.");
	TestFramework::RequireEqual(events[2], std::string("overlay.Draw"), "Overlay should draw last.");
	manager.Finalize();
}

