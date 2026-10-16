#include "tests/TestFramework.h"

#include "engine/object/GameObject.h"

namespace {
class CountingComponent final : public Component {
  public:
	int updates = 0;
	int fixedUpdates = 0;
	int draws = 0;

  private:
	void OnUpdate() override { ++updates; }
	void OnFixedUpdate() override { ++fixedUpdates; }
	void OnDraw() override { ++draws; }
};
} // namespace

TEST_CASE("GameObject dispatches callbacks only to active enabled components") {
	GameObject object("Counter");
	CountingComponent &component = object.AddComponent<CountingComponent>();
	object.Update();
	object.FixedUpdate();
	object.Draw();
	TestFramework::RequireEqual(component.updates, 1, "Update callback count is incorrect.");
	TestFramework::RequireEqual(component.fixedUpdates, 1, "FixedUpdate callback count is incorrect.");
	TestFramework::RequireEqual(component.draws, 1, "Draw callback count is incorrect.");

	component.SetEnabled(false);
	object.Update();
	object.Draw();
	TestFramework::RequireEqual(component.updates, 1, "Disabled component was updated.");
	TestFramework::RequireEqual(component.draws, 1, "Disabled component was drawn.");
}

