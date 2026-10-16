#pragma once

/// <summary>
/// x成分とy成分を持つ二次元ベクトル。
/// </summary>
struct Vector2 {
	float x; // 水平方向の成分
	float y; // 垂直方向の成分
};

/// <summary>
/// x成分、y成分、z成分を持つ三次元ベクトル。
/// </summary>
struct Vector3 {
	float x; // X軸方向の成分
	float y; // Y軸方向の成分
	float z; // Z軸方向の成分
};

/// <summary>
/// x成分、y成分、z成分、w成分を持つ四次元ベクトル。
/// </summary>
struct Vector4 {
	float x; // X成分または赤成分
	float y; // Y成分または緑成分
	float z; // Z成分または青成分
	float w; // W成分またはアルファ成分
};

/// <summary>
/// 行優先の4行4列で値を保持する変換行列。
/// </summary>
struct Matrix4x4 {
	float m[4][4]; // 行番号と列番号で参照する16個の行列要素
};

/// <summary>
/// 拡縮、オイラー回転、平行移動をまとめた姿勢データ。
/// </summary>
struct Transform {
	Vector3 scale;     // 各軸の拡縮率
	Vector3 rotate;    // 各軸周りのオイラー回転角
	Vector3 translate; // 三次元空間上の平行移動量
};
