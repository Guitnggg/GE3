#include "tests/TestFramework.h"

#include "engine/object/TransformNode.h"

TEST_CASE("TransformNode propagates parent movement to children") {
	TransformNode parent;
	TransformNode child;
	parent.SetLocalPosition({10.0f, 2.0f, -3.0f});
	child.SetLocalPosition({1.0f, 4.0f, 2.0f});
	child.SetParent(&parent);

	const Vector3 world = child.GetWorldPosition();
	TestFramework::RequireNear(world.x, 11.0f, 0.0001f, "Child world X is incorrect.");
	TestFramework::RequireNear(world.y, 6.0f, 0.0001f, "Child world Y is incorrect.");
	TestFramework::RequireNear(world.z, -1.0f, 0.0001f, "Child world Z is incorrect.");
}

TEST_CASE("TransformNode rejects hierarchy cycles") {
	TransformNode parent;
	TransformNode child;
	child.SetParent(&parent);
	TestFramework::RequireThrows<std::invalid_argument>([&] { parent.SetParent(&child); },
	                                                    "Transform hierarchy cycle was accepted.");
}

