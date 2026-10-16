#include "tests/TestFramework.h"

#include "engine/collision/CollisionWorld.h"

#include <vector>

TEST_CASE("CollisionWorld emits enter stay and exit in order") {
	CollisionWorld world;
	std::vector<CollisionEventType> events;
	SphereColliderDesc first{};
	first.shape = {{0.0f, 0.0f, 0.0f}, 1.0f};
	first.callback = [&](const CollisionEvent &event) { events.push_back(event.type); };
	const ColliderHandle firstHandle = world.RegisterSphere(first);
	SphereColliderDesc second{};
	second.shape = {{1.0f, 0.0f, 0.0f}, 1.0f};
	const ColliderHandle secondHandle = world.RegisterSphere(second);

	world.Update();
	world.Update();
	world.SetSphere(secondHandle, {{4.0f, 0.0f, 0.0f}, 1.0f});
	world.Update();
	TestFramework::RequireEqual(events.size(), size_t{3}, "Unexpected collision event count.");
	TestFramework::RequireEqual(events[0], CollisionEventType::Enter, "First event was not Enter.");
	TestFramework::RequireEqual(events[1], CollisionEventType::Stay, "Second event was not Stay.");
	TestFramework::RequireEqual(events[2], CollisionEventType::Exit, "Third event was not Exit.");
	TestFramework::Require(world.IsRegistered(firstHandle), "Registered collider disappeared unexpectedly.");
}

TEST_CASE("CollisionWorld applies layer masks") {
	CollisionWorld world;
	int eventCount = 0;
	SphereColliderDesc first{};
	first.shape = {{}, 1.0f};
	first.layer = 1u;
	first.mask = 1u;
	first.callback = [&](const CollisionEvent &) { ++eventCount; };
	SphereColliderDesc second{};
	second.shape = {{}, 1.0f};
	second.layer = 2u;
	second.mask = 2u;
	(void)world.RegisterSphere(first);
	(void)world.RegisterSphere(second);
	world.Update();
	TestFramework::RequireEqual(eventCount, 0, "Filtered colliders generated an event.");
}
