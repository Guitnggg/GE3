#pragma once

/// <summary>
/// x成分とy成分を持つ二次元ベクトル。
/// </summary>
struct Vector2 {
	float x;
	float y;
};

/// <summary>
/// x成分、y成分、z成分を持つ三次元ベクトル。
/// </summary>
struct Vector3 {
	float x;
	float y;
	float z;
};

/// <summary>
/// x成分、y成分、z成分、w成分を持つ四次元ベクトル。
/// </summary>
struct Vector4 {
	float x;
	float y;
	float z;
	float s;
};

/// <summary>
/// 行優先の4行4列で値を保持する変換行列。
/// </summary>
struct Matrix4x4 {
	float m[4][4];
};

/// <summary>
/// 拡縮、オイラー回転、平行移動をまとめた姿勢データ。
/// </summary>
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};
