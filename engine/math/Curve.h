#pragma once

#include "engine/math/MathUtility.h"

namespace Curve {
	inline Vector3 QuadraticBezier(const Vector3& start, const Vector3& control,
		const Vector3& end, float t) {
		t = std::clamp(t, 0.0f, 1.0f);
		return Lerp(Lerp(start, control, t), Lerp(control, end, t), t);
	}

	inline Vector3 CubicBezier(const Vector3& start, const Vector3& control1,
		const Vector3& control2, const Vector3& end, float t) {
		t = std::clamp(t, 0.0f, 1.0f);
		const Vector3 first = Lerp(start, control1, t);
		const Vector3 second = Lerp(control1, control2, t);
		const Vector3 third = Lerp(control2, end, t);
		return Lerp(Lerp(first, second, t), Lerp(second, third, t), t);
	}

	inline Vector3 CatmullRom(const Vector3& p0, const Vector3& p1,
		const Vector3& p2, const Vector3& p3, float t) {
		t = std::clamp(t, 0.0f, 1.0f);
		const float t2 = t * t;
		const float t3 = t2 * t;
		return {
			0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t +
				(2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
				(-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3),
			0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t +
				(2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
				(-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3),
			0.5f * ((2.0f * p1.z) + (-p0.z + p2.z) * t +
				(2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * t2 +
				(-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * t3),
		};
	}
}
