#include "tests/TestFramework.h"

#include "engine/scene/SceneManager.h"
#include "engine/scene/Scene.h"
#include "engine/scene/AbstractSceneFactory.h"

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

class ThrowingScene final : public IScene {
  public:
	void Initialize(const SceneContext &) override { throw std::runtime_error("Initialization failed."); }
	void Update() override {}
	void FixedUpdate() override {}
	void Draw() override {}
	void Finalize() override {}
};

class RecordingSceneFactory final : public AbstractSceneFactory {
  public:
	explicit RecordingSceneFactory(std::vector<std::string> &events) : events_(events) {}

	std::unique_ptr<IScene> CreateScene(std::string_view sceneId) override {
		return std::make_unique<RecordingScene>(events_, std::string(sceneId));
	}

  private:
	std::vector<std::string> &events_;
};

class ExternalCounterComponent final : public Component {
  public:
	explicit ExternalCounterComponent(int &fixedUpdates) : fixedUpdates_(fixedUpdates) {}

  private:
	void OnFixedUpdate() override { ++fixedUpdates_; }
	int &fixedUpdates_;
};

class DestroyDuringFixedScene final : public Scene {
  public:
	explicit DestroyDuringFixedScene(int &fixedUpdates) : fixedUpdates_(fixedUpdates) {}

  private:
	void OnInitialize() override {
		object_ = &CreateGameObject("PendingDestroy");
		object_->AddComponent<ExternalCounterComponent>(fixedUpdates_);
	}
	void OnFixedUpdate() override {
		if (object_ != nullptr) {
			DestroyGameObject(*object_);
			object_ = nullptr;
		}
	}

	int &fixedUpdates_;
	GameObject *object_ = nullptr;
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
	TestFramework::RequireEqual(events[2], std::string("second.Initialize"), "Next scene was not initialized first.");
	TestFramework::RequireEqual(events[3], std::string("first.Finalize"), "Previous scene was not finalized.");
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

TEST_CASE("SceneManager preserves the current scene when replacement initialization fails") {
	std::vector<std::string> events;
	SceneManager manager;
	manager.Initialize({}, std::make_unique<RecordingScene>(events, "current"));
	manager.ChangeScene(std::make_unique<ThrowingScene>());
	TestFramework::RequireThrows<std::runtime_error>([&] { manager.BeginFrame(); },
	                                                 "Failing replacement did not propagate its error.");

	manager.Update();
	TestFramework::RequireEqual(events.back(), std::string("current.Update"),
	                            "Current scene was lost after replacement failure.");
	manager.Finalize();
}

TEST_CASE("Scene stops a GameObject immediately after a fixed-update destroy request") {
	int componentFixedUpdates = 0;
	DestroyDuringFixedScene scene(componentFixedUpdates);
	scene.Initialize({});
	scene.FixedUpdate();
	TestFramework::RequireEqual(componentFixedUpdates, 0,
	                            "Pending-destroy GameObject received a FixedUpdate callback.");
	scene.FixedUpdate();
	TestFramework::RequireEqual(componentFixedUpdates, 0, "Destroyed GameObject survived into the next fixed step.");
	scene.Finalize();
}

TEST_CASE("SceneManager creates initial and replacement scenes through its factory") {
	std::vector<std::string> events;
	SceneManager manager;
	manager.SetSceneFactory(std::make_unique<RecordingSceneFactory>(events));
	manager.Initialize({}, "TITLE");
	manager.ChangeScene("GAME");
	manager.BeginFrame();

	TestFramework::RequireEqual(events[0], std::string("TITLE.Initialize"), "Factory did not create title scene.");
	TestFramework::RequireEqual(events[1], std::string("GAME.Initialize"), "Factory did not create game scene.");
	TestFramework::RequireEqual(events[2], std::string("TITLE.Finalize"), "Old scene was not finalized.");
	manager.Finalize();
}

TEST_CASE("SceneManager exposes itself through SceneContext") {
	class ContextScene final : public Scene {
	  public:
		SceneManager *manager = nullptr;

	  private:
		void OnInitialize() override { manager = GetContext().sceneManager; }
	};

	auto scene = std::make_unique<ContextScene>();
	ContextScene *scenePointer = scene.get();
	SceneManager manager;
	manager.Initialize({}, std::move(scene));
	TestFramework::Require(scenePointer->manager == &manager, "SceneContext did not expose its owning SceneManager.");
	manager.Finalize();
}
