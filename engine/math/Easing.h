#pragma once

#include "engine/math/MathUtility.h"

#include <algorithm>
#include <cmath>

enum class EasingType {
	Linear,
	SineIn,
	SineOut,
	SineInOut,
	QuadIn,
	QuadOut,
	QuadInOut,
	CubicIn,
	CubicOut,
	CubicInOut,
	BackIn,
	BackOut,
	BackInOut,
	BounceIn,
	BounceOut,
	BounceInOut,
	ElasticIn,
	ElasticOut,
	ElasticInOut,
};

/// <summary>0～1の進行度を演出向けの曲線へ変換する関数群。</summary>
namespace Easing {
/// <summary>補間係数を0から1の範囲へ制限する。</summary>
inline float Clamp01(float t) {
	return std::clamp(t, 0.0f, 1.0f);
}
/// <summary>一定速度で変化する補間係数を返す。</summary>
inline float Linear(float t) {
	return Clamp01(t);
}

/// <summary>ゆっくり開始する正弦曲線の補間係数を返す。</summary>
inline float SineIn(float t) {
	constexpr float kHalfPi = 1.57079632679489661923f;
	return 1.0f - std::cos(Clamp01(t) * kHalfPi);
}
/// <summary>ゆっくり停止する正弦曲線の補間係数を返す。</summary>
inline float SineOut(float t) {
	constexpr float kHalfPi = 1.57079632679489661923f;
	return std::sin(Clamp01(t) * kHalfPi);
}
/// <summary>開始と停止が滑らかな正弦曲線の補間係数を返す。</summary>
inline float SineInOut(float t) {
	constexpr float kPi = 3.14159265358979323846f;
	return -(std::cos(kPi * Clamp01(t)) - 1.0f) * 0.5f;
}

/// <summary>ゆっくり開始する二次曲線の補間係数を返す。</summary>
inline float QuadIn(float t) {
	t = Clamp01(t);
	return t * t;
}
/// <summary>ゆっくり停止する二次曲線の補間係数を返す。</summary>
inline float QuadOut(float t) {
	t = Clamp01(t);
	return 1.0f - (1.0f - t) * (1.0f - t);
}
/// <summary>開始と停止が滑らかな二次曲線の補間係数を返す。</summary>
inline float QuadInOut(float t) {
	t = Clamp01(t);
	return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
}

/// <summary>ゆっくり開始する三次曲線の補間係数を返す。</summary>
inline float CubicIn(float t) {
	t = Clamp01(t);
	return t * t * t;
}
/// <summary>ゆっくり停止する三次曲線の補間係数を返す。</summary>
inline float CubicOut(float t) {
	t = 1.0f - Clamp01(t);
	return 1.0f - t * t * t;
}
/// <summary>開始と停止が滑らかな三次曲線の補間係数を返す。</summary>
inline float CubicInOut(float t) {
	t = Clamp01(t);
	return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
}

/// <summary>三次多項式による滑らかな補間係数を返す。</summary>
inline float SmoothStep(float t) {
	t = Clamp01(t);
	return t * t * (3.0f - 2.0f * t);
}
/// <summary>五次多項式によるさらに滑らかな補間係数を返す。</summary>
inline float SmootherStep(float t) {
	t = Clamp01(t);
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

/// <summary>開始時に反対方向へ少し戻る補間係数を返す。</summary>
inline float BackIn(float t) {
	constexpr float kOvershoot = 1.70158f;
	t = Clamp01(t);
	return (kOvershoot + 1.0f) * t * t * t - kOvershoot * t * t;
}
/// <summary>終了値を一度越えて戻る補間係数を返す。</summary>
inline float BackOut(float t) {
	constexpr float kOvershoot = 1.70158f;
	t = Clamp01(t) - 1.0f;
	return 1.0f + (kOvershoot + 1.0f) * t * t * t + kOvershoot * t * t;
}
/// <summary>開始と終了で行き過ぎを加える補間係数を返す。</summary>
inline float BackInOut(float t) {
	constexpr float kOvershoot = 1.70158f * 1.525f;
	t = Clamp01(t);
	if (t < 0.5f) {
		const float doubled = 2.0f * t;
		return doubled * doubled * ((kOvershoot + 1.0f) * doubled - kOvershoot) * 0.5f;
	}
	const float doubled = 2.0f * t - 2.0f;
	return (doubled * doubled * ((kOvershoot + 1.0f) * doubled + kOvershoot) + 2.0f) * 0.5f;
}

/// <summary>終了付近で跳ねる補間係数を返す。</summary>
inline float BounceOut(float t) {
	constexpr float kBounce = 7.5625f;
	constexpr float kSection = 2.75f;
	t = Clamp01(t);
	if (t < 1.0f / kSection) {
		return kBounce * t * t;
	}
	if (t < 2.0f / kSection) {
		t -= 1.5f / kSection;
		return kBounce * t * t + 0.75f;
	}
	if (t < 2.5f / kSection) {
		t -= 2.25f / kSection;
		return kBounce * t * t + 0.9375f;
	}
	t -= 2.625f / kSection;
	return kBounce * t * t + 0.984375f;
}
/// <summary>開始付近で跳ねる補間係数を返す。</summary>
inline float BounceIn(float t) {
	return 1.0f - BounceOut(1.0f - Clamp01(t));
}
/// <summary>開始と終了で跳ねる補間係数を返す。</summary>
inline float BounceInOut(float t) {
	t = Clamp01(t);
	return t < 0.5f ? (1.0f - BounceOut(1.0f - 2.0f * t)) * 0.5f : (1.0f + BounceOut(2.0f * t - 1.0f)) * 0.5f;
}

/// <summary>開始側で振動する補間係数を返す。</summary>
inline float ElasticIn(float t) {
	constexpr float kTau = 6.28318530717958647692f;
	t = Clamp01(t);
	if (t == 0.0f || t == 1.0f) {
		return t;
	}
	return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((10.0f * t - 10.75f) * kTau / 3.0f);
}
/// <summary>終了側で振動する補間係数を返す。</summary>
inline float ElasticOut(float t) {
	constexpr float kTau = 6.28318530717958647692f;
	t = Clamp01(t);
	if (t == 0.0f || t == 1.0f) {
		return t;
	}
	return std::pow(2.0f, -10.0f * t) * std::sin((10.0f * t - 0.75f) * kTau / 3.0f) + 1.0f;
}
/// <summary>開始と終了で振動する補間係数を返す。</summary>
inline float ElasticInOut(float t) {
	constexpr float kTau = 6.28318530717958647692f;
	t = Clamp01(t);
	if (t == 0.0f || t == 1.0f) {
		return t;
	}
	const float sine = std::sin((20.0f * t - 11.125f) * kTau / 4.5f);
	return t < 0.5f ? -std::pow(2.0f, 20.0f * t - 10.0f) * sine * 0.5f
	                : std::pow(2.0f, -20.0f * t + 10.0f) * sine * 0.5f + 1.0f;
}

/// <summary>列挙値で指定されたイージング関数を評価する。</summary>
inline float Evaluate(EasingType type, float t) {
	switch (type) {
	case EasingType::Linear:
		return Linear(t);
	case EasingType::SineIn:
		return SineIn(t);
	case EasingType::SineOut:
		return SineOut(t);
	case EasingType::SineInOut:
		return SineInOut(t);
	case EasingType::QuadIn:
		return QuadIn(t);
	case EasingType::QuadOut:
		return QuadOut(t);
	case EasingType::QuadInOut:
		return QuadInOut(t);
	case EasingType::CubicIn:
		return CubicIn(t);
	case EasingType::CubicOut:
		return CubicOut(t);
	case EasingType::CubicInOut:
		return CubicInOut(t);
	case EasingType::BackIn:
		return BackIn(t);
	case EasingType::BackOut:
		return BackOut(t);
	case EasingType::BackInOut:
		return BackInOut(t);
	case EasingType::BounceIn:
		return BounceIn(t);
	case EasingType::BounceOut:
		return BounceOut(t);
	case EasingType::BounceInOut:
		return BounceInOut(t);
	case EasingType::ElasticIn:
		return ElasticIn(t);
	case EasingType::ElasticOut:
		return ElasticOut(t);
	case EasingType::ElasticInOut:
		return ElasticInOut(t);
	}
	return Linear(t);
}
} // namespace Easing

/// <summary>指定したイージングで実数値を補間する。</summary>
inline float EaseLerp(float start, float end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}

/// <summary>指定したイージングで二次元値を補間する。</summary>
inline Vector2 EaseLerp(const Vector2 &start, const Vector2 &end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}

/// <summary>指定したイージングで三次元値を補間する。</summary>
inline Vector3 EaseLerp(const Vector3 &start, const Vector3 &end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}

/// <summary>指定したイージングで四次元値を補間する。</summary>
inline Vector4 EaseLerp(const Vector4 &start, const Vector4 &end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}
