#pragma once

#include "engine/collision/CollisionWorld.h"

namespace GameCollisionLayers {
	inline constexpr CollisionLayer Enemy = 1u << 0;
	inline constexpr CollisionLayer PlayerProjectile = 1u << 1;
}
