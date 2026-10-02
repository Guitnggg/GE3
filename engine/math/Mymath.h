#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 {
	float x;
	float y;
};

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 {
	float x;
	float y;
	float z;
};

/// <summary>
/// 4次元ベクトル
/// </summary>
struct Vector4 {
	float x;
	float y;
	float z;
	float s;
};

/// <summary>
/// 4x4行列
/// </summary>
struct Matrix4x4 {
	float m[4][4];
};

/// <summary>
/// 4x4単位行列を作成する
/// </summary>
inline Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

/// <summary>
/// 拡縮行列を作成する
/// </summary>
inline Matrix4x4 MakeScaleMatrix(Vector3 scale) {
	Matrix4x4 result{};
	result.m[0][0] = scale.x;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = scale.y;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = scale.z;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

/// <summary>
/// Z軸回転行列を作成する
/// </summary>
inline Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result{};
	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = -(std::sin(radian));
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

/// <summary>
/// 平行移動行列を作成する
/// </summary>
inline Matrix4x4 MakeTranslateMatrix(Vector3 translate) {
	Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

/// <summary>
/// 座標変換に使用する拡縮・回転・平行移動データ
/// </summary>
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

#pragma region Affine

/// <summary>
/// 4x4行列同士を乗算する
/// </summary>
inline Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2) {
	Matrix4x4 result{};
	result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
	result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
	result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
	result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];

	result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
	result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
	result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
	result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];

	result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
	result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
	result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
	result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];

	result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
	result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
	result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
	result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];
	return result;
}

/// <summary>
/// 拡縮・回転・平行移動をまとめたアフィン変換行列を作成する
/// </summary>
inline Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {

	Matrix4x4 resultX{};
	resultX.m[0][0] = 1.0f;
	resultX.m[0][1] = 0.0f;
	resultX.m[0][2] = 0.0f;
	resultX.m[0][3] = 0.0f;
	resultX.m[1][0] = 0.0f;
	resultX.m[1][1] = std::cos(rotate.x);
	resultX.m[1][2] = std::sin(rotate.x);
	resultX.m[1][3] = 0.0f;
	resultX.m[2][0] = 0.0f;
	resultX.m[2][1] = -(std::sin(rotate.x));
	resultX.m[2][2] = std::cos(rotate.x);
	resultX.m[2][3] = 0.0f;
	resultX.m[3][0] = 0.0f;
	resultX.m[3][1] = 0.0f;
	resultX.m[3][2] = 0.0f;
	resultX.m[3][3] = 1.0f;

	Matrix4x4 resultY{};
	resultY.m[0][0] = std::cos(rotate.y);
	resultY.m[0][1] = 0.0f;
	resultY.m[0][2] = -(std::sin(rotate.y));
	resultY.m[0][3] = 0.0f;
	resultY.m[1][0] = 0.0f;
	resultY.m[1][1] = 1.0f;
	resultY.m[1][2] = 0.0f;
	resultY.m[1][3] = 0.0f;
	resultY.m[2][0] = std::sin(rotate.y);
	resultY.m[2][1] = 0.0f;
	resultY.m[2][2] = std::cos(rotate.y);
	resultY.m[2][3] = 0.0f;
	resultY.m[3][0] = 0.0f;
	resultY.m[3][1] = 0.0f;
	resultY.m[3][2] = 0.0f;
	resultY.m[3][3] = 1.0f;

	Matrix4x4 resultZ{};
	resultZ.m[0][0] = std::cos(rotate.z);
	resultZ.m[0][1] = std::sin(rotate.z);
	resultZ.m[0][2] = 0.0f;
	resultZ.m[0][3] = 0.0f;
	resultZ.m[1][0] = -(std::sin(rotate.z));
	resultZ.m[1][1] = std::cos(rotate.z);
	resultZ.m[1][2] = 0.0f;
	resultZ.m[1][3] = 0.0f;
	resultZ.m[2][0] = 0.0f;
	resultZ.m[2][1] = 0.0f;
	resultZ.m[2][2] = 1.0f;
	resultZ.m[2][3] = 0.0f;
	resultZ.m[3][0] = 0.0f;
	resultZ.m[3][1] = 0.0f;
	resultZ.m[3][2] = 0.0f;
	resultZ.m[3][3] = 1.0f;

	Matrix4x4 rotateXYZ = Multiply(resultX, Multiply(resultY, resultZ));

	Matrix4x4 result;
	result.m[0][0] = scale.x * rotateXYZ.m[0][0];
	result.m[0][1] = scale.x * rotateXYZ.m[0][1];
	result.m[0][2] = scale.x * rotateXYZ.m[0][2];
	result.m[0][3] = 0.0f;
	result.m[1][0] = scale.y * rotateXYZ.m[1][0];
	result.m[1][1] = scale.y * rotateXYZ.m[1][1];
	result.m[1][2] = scale.y * rotateXYZ.m[1][2];
	result.m[1][3] = 0.0f;
	result.m[2][0] = scale.z * rotateXYZ.m[2][0];
	result.m[2][1] = scale.z * rotateXYZ.m[2][1];
	result.m[2][2] = scale.z * rotateXYZ.m[2][2];
	result.m[2][3] = 0.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}
#pragma endregion

#pragma region 逆数
/// <summary>
/// 4x4行列の逆行列を作成する
/// </summary>
inline Matrix4x4 Inverse(const Matrix4x4& m) {
	float A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]
		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];


	Matrix4x4 result{};
	result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[1][1] * m.m[2][3] * m.m[3][2]) / A;

	result.m[0][1] = (-m.m[0][1] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[2][3] * m.m[3][2]) / A;

	result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[3][2]) / A;

	result.m[0][3] = (-m.m[0][1] * m.m[1][2] * m.m[2][3]
		- m.m[0][2] * m.m[1][3] * m.m[2][1]
		- m.m[0][3] * m.m[1][1] * m.m[2][2]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2]) / A;


	result.m[1][0] = (-m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[1][3] * m.m[2][0] * m.m[3][2]
		+ m.m[1][3] * m.m[2][2] * m.m[3][0]
		+ m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[1][0] * m.m[2][3] * m.m[3][2]) / A;

	result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3]
		+ m.m[0][2] * m.m[2][3] * m.m[3][0]
		+ m.m[0][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[2][2] * m.m[3][0]
		- m.m[0][2] * m.m[2][0] * m.m[3][3]
		- m.m[0][0] * m.m[2][3] * m.m[3][2]) / A;

	result.m[1][2] = (-m.m[0][0] * m.m[1][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][3] * m.m[3][0]
		- m.m[0][3] * m.m[1][0] * m.m[3][2]
		+ m.m[0][3] * m.m[1][2] * m.m[3][0]
		+ m.m[0][2] * m.m[1][0] * m.m[3][3]
		+ m.m[0][0] * m.m[1][3] * m.m[3][2]) / A;

	result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0]
		- m.m[0][2] * m.m[1][0] * m.m[2][3]
		- m.m[0][0] * m.m[1][3] * m.m[2][2]) / A;


	result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[1][3] * m.m[2][0] * m.m[3][1]
		- m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[1][0] * m.m[2][3] * m.m[3][1]) / A;

	result.m[2][1] = (-m.m[0][0] * m.m[2][1] * m.m[3][3]
		- m.m[0][1] * m.m[2][3] * m.m[3][0]
		- m.m[0][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[2][1] * m.m[3][0]
		+ m.m[0][1] * m.m[2][0] * m.m[3][3]
		+ m.m[0][0] * m.m[2][3] * m.m[3][1]) / A;

	result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][3] * m.m[3][0]
		+ m.m[0][3] * m.m[1][0] * m.m[3][1]
		- m.m[0][3] * m.m[1][1] * m.m[3][0]
		- m.m[0][1] * m.m[1][0] * m.m[3][3]
		- m.m[0][0] * m.m[1][3] * m.m[3][1]) / A;

	result.m[2][3] = (-m.m[0][0] * m.m[1][1] * m.m[2][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0]
		- m.m[0][3] * m.m[1][0] * m.m[2][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1]) / A;


	result.m[3][0] = (-m.m[1][0] * m.m[2][1] * m.m[3][2]
		- m.m[1][1] * m.m[2][2] * m.m[3][0]
		- m.m[1][2] * m.m[2][0] * m.m[3][1]
		+ m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[1][1] * m.m[2][0] * m.m[3][2]
		+ m.m[1][0] * m.m[2][2] * m.m[3][1]) / A;

	result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[2][1] * m.m[3][0]
		- m.m[0][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][0] * m.m[2][2] * m.m[3][1]) / A;

	result.m[3][2] = (-m.m[0][0] * m.m[1][1] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[3][0]
		- m.m[0][2] * m.m[1][0] * m.m[3][1]
		+ m.m[0][2] * m.m[1][1] * m.m[3][0]
		+ m.m[0][1] * m.m[1][0] * m.m[3][2]
		+ m.m[0][0] * m.m[1][2] * m.m[3][1]) / A;

	result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0]
		- m.m[0][1] * m.m[1][0] * m.m[2][2]
		- m.m[0][0] * m.m[1][2] * m.m[2][1]) / A;

	return result;
}
#pragma endregion

/// <summary>
/// 透視投影行列を作成する
/// </summary>
inline Matrix4x4 MakePerspectiveFovMatrix(float forY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result;
	float cot = 1 / std::tan(forY / 2);

	result.m[0][0] = (1 / aspectRatio) * cot;
	result.m[1][1] = cot;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);

	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][3] = 0.0f;

	return result;
}

/// <summary>
/// 正射影行列を作成する
/// </summary>
inline Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result;
	result.m[0][0] = 2 / (right - left);
	result.m[1][1] = 2 / (top - bottom);
	result.m[2][2] = 1 / (farClip - nearClip);

	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][3] = 0.0f;

	return result;
}

/// <summary>
/// 頂点シェーダーへ渡す頂点データ
/// </summary>
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

/// <summary>
/// 描画用マテリアルデータ
/// </summary>
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];  // 定数バッファのアライメント調整用
	Matrix4x4 uvTransform;
};

/// <summary>
/// マテリアルファイルから読み込んだデータ
/// </summary>
struct MaterialData {
	std::string textureFilePath;
};

/// <summary>
/// シェーダーへ渡す座標変換行列
/// </summary>
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

/// <summary>
/// 平行光源データ
/// </summary>
struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

/// <summary>
/// 3次元ベクトルを正規化する
/// </summary>
inline Vector3 Normalize(const Vector3& v) {
	const float lengthSquared = v.x * v.x + v.y * v.y + v.z * v.z;
	constexpr float kLengthSquaredEpsilon = 1.0e-12f;
	if (lengthSquared <= kLengthSquaredEpsilon) {
		return {};
	}
	const float inverseLength = 1.0f / std::sqrt(lengthSquared);
	return {v.x * inverseLength, v.y * inverseLength, v.z * inverseLength};
}

/// <summary>2値を割合tで線形補間する。tは範囲外でもそのまま外挿する。</summary>
inline float Lerp(float start, float end, float t) {
	return start + (end - start) * t;
}

inline Vector2 Lerp(const Vector2& start, const Vector2& end, float t) {
	return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t)};
}

inline Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
	return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t), Lerp(start.z, end.z, t)};
}

inline Vector4 Lerp(const Vector4& start, const Vector4& end, float t) {
	return {Lerp(start.x, end.x, t), Lerp(start.y, end.y, t),
		Lerp(start.z, end.z, t), Lerp(start.s, end.s, t)};
}

/// <summary>valueがstartからendまでのどの割合にあるかを0～1で返す。</summary>
inline float InverseLerp(float start, float end, float value) {
	if (std::abs(end - start) <= 1.0e-6f) { return 0.0f; }
	return std::clamp((value - start) / (end - start), 0.0f, 1.0f);
}

/// <summary>入力範囲の値を出力範囲へ線形変換する。</summary>
inline float Remap(float inputStart, float inputEnd, float outputStart, float outputEnd, float value) {
	return Lerp(outputStart, outputEnd, InverseLerp(inputStart, inputEnd, value));
}

/// <summary>現在値をtargetへ最大maxDeltaだけ近づける。</summary>
inline float MoveTowards(float current, float target, float maxDelta) {
	if (maxDelta < 0.0f) { return current; }
	const float difference = target - current;
	if (std::abs(difference) <= maxDelta) { return target; }
	return current + std::copysign(maxDelta, difference);
}

/// <summary>valueを0以上length未満の周期へ折り返す。</summary>
inline float Repeat(float value, float length) {
	if (!std::isfinite(value) || !std::isfinite(length) || length <= 0.0f) { return 0.0f; }
	return value - std::floor(value / length) * length;
}

/// <summary>0からlengthまでを往復する値を返す。</summary>
inline float PingPong(float value, float length) {
	const float repeated = Repeat(value, length * 2.0f);
	return length - std::abs(repeated - length);
}

/// <summary>ラジアン角を-pi以上pi以下へ正規化する。</summary>
inline float NormalizeAngle(float radians) {
	constexpr float kPi = 3.14159265358979323846f;
	constexpr float kTau = kPi * 2.0f;
	float normalized = Repeat(radians + kPi, kTau) - kPi;
	return normalized == -kPi ? kPi : normalized;
}

/// <summary>currentからtargetまでの最短角度差をラジアンで返す。</summary>
inline float DeltaAngle(float current, float target) {
	return NormalizeAngle(target - current);
}

/// <summary>2つのラジアン角を最短方向へ補間する。</summary>
inline float LerpAngle(float start, float end, float t) {
	return start + DeltaAngle(start, end) * std::clamp(t, 0.0f, 1.0f);
}

/// <summary>現在角をtargetへ最大maxDeltaラジアンだけ最短方向へ近づける。</summary>
inline float MoveTowardsAngle(float current, float target, float maxDelta) {
	const float delta = DeltaAngle(current, target);
	if (std::abs(delta) <= maxDelta) { return target; }
	return current + std::copysign(std::max(0.0f, maxDelta), delta);
}

enum class EasingType {
	Linear,
	SineIn, SineOut, SineInOut,
	QuadIn, QuadOut, QuadInOut,
	CubicIn, CubicOut, CubicInOut,
	BackIn, BackOut, BackInOut,
	BounceIn, BounceOut, BounceInOut,
	ElasticIn, ElasticOut, ElasticInOut,
};

/// <summary>0～1の進行度を演出向けの曲線へ変換する関数群。</summary>
namespace Easing {
	inline float Clamp01(float t) { return std::clamp(t, 0.0f, 1.0f); }
	inline float Linear(float t) { return Clamp01(t); }

	inline float SineIn(float t) {
		constexpr float kHalfPi = 1.57079632679489661923f;
		return 1.0f - std::cos(Clamp01(t) * kHalfPi);
	}
	inline float SineOut(float t) {
		constexpr float kHalfPi = 1.57079632679489661923f;
		return std::sin(Clamp01(t) * kHalfPi);
	}
	inline float SineInOut(float t) {
		constexpr float kPi = 3.14159265358979323846f;
		return -(std::cos(kPi * Clamp01(t)) - 1.0f) * 0.5f;
	}

	inline float QuadIn(float t) { t = Clamp01(t); return t * t; }
	inline float QuadOut(float t) { t = Clamp01(t); return 1.0f - (1.0f - t) * (1.0f - t); }
	inline float QuadInOut(float t) {
		t = Clamp01(t);
		return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
	}

	inline float CubicIn(float t) { t = Clamp01(t); return t * t * t; }
	inline float CubicOut(float t) { t = 1.0f - Clamp01(t); return 1.0f - t * t * t; }
	inline float CubicInOut(float t) {
		t = Clamp01(t);
		return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
	}

	inline float SmoothStep(float t) { t = Clamp01(t); return t * t * (3.0f - 2.0f * t); }
	inline float SmootherStep(float t) { t = Clamp01(t); return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

	inline float BackIn(float t) {
		constexpr float kOvershoot = 1.70158f;
		t = Clamp01(t);
		return (kOvershoot + 1.0f) * t * t * t - kOvershoot * t * t;
	}
	inline float BackOut(float t) {
		constexpr float kOvershoot = 1.70158f;
		t = Clamp01(t) - 1.0f;
		return 1.0f + (kOvershoot + 1.0f) * t * t * t + kOvershoot * t * t;
	}
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

	inline float BounceOut(float t) {
		constexpr float kBounce = 7.5625f;
		constexpr float kSection = 2.75f;
		t = Clamp01(t);
		if (t < 1.0f / kSection) { return kBounce * t * t; }
		if (t < 2.0f / kSection) { t -= 1.5f / kSection; return kBounce * t * t + 0.75f; }
		if (t < 2.5f / kSection) { t -= 2.25f / kSection; return kBounce * t * t + 0.9375f; }
		t -= 2.625f / kSection;
		return kBounce * t * t + 0.984375f;
	}
	inline float BounceIn(float t) { return 1.0f - BounceOut(1.0f - Clamp01(t)); }
	inline float BounceInOut(float t) {
		t = Clamp01(t);
		return t < 0.5f ? (1.0f - BounceOut(1.0f - 2.0f * t)) * 0.5f
			: (1.0f + BounceOut(2.0f * t - 1.0f)) * 0.5f;
	}

	inline float ElasticIn(float t) {
		constexpr float kTau = 6.28318530717958647692f;
		t = Clamp01(t);
		if (t == 0.0f || t == 1.0f) { return t; }
		return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((10.0f * t - 10.75f) * kTau / 3.0f);
	}
	inline float ElasticOut(float t) {
		constexpr float kTau = 6.28318530717958647692f;
		t = Clamp01(t);
		if (t == 0.0f || t == 1.0f) { return t; }
		return std::pow(2.0f, -10.0f * t) * std::sin((10.0f * t - 0.75f) * kTau / 3.0f) + 1.0f;
	}
	inline float ElasticInOut(float t) {
		constexpr float kTau = 6.28318530717958647692f;
		t = Clamp01(t);
		if (t == 0.0f || t == 1.0f) { return t; }
		const float sine = std::sin((20.0f * t - 11.125f) * kTau / 4.5f);
		return t < 0.5f ? -std::pow(2.0f, 20.0f * t - 10.0f) * sine * 0.5f
			: std::pow(2.0f, -20.0f * t + 10.0f) * sine * 0.5f + 1.0f;
	}

	inline float Evaluate(EasingType type, float t) {
		switch (type) {
		case EasingType::Linear: return Linear(t);
		case EasingType::SineIn: return SineIn(t);
		case EasingType::SineOut: return SineOut(t);
		case EasingType::SineInOut: return SineInOut(t);
		case EasingType::QuadIn: return QuadIn(t);
		case EasingType::QuadOut: return QuadOut(t);
		case EasingType::QuadInOut: return QuadInOut(t);
		case EasingType::CubicIn: return CubicIn(t);
		case EasingType::CubicOut: return CubicOut(t);
		case EasingType::CubicInOut: return CubicInOut(t);
		case EasingType::BackIn: return BackIn(t);
		case EasingType::BackOut: return BackOut(t);
		case EasingType::BackInOut: return BackInOut(t);
		case EasingType::BounceIn: return BounceIn(t);
		case EasingType::BounceOut: return BounceOut(t);
		case EasingType::BounceInOut: return BounceInOut(t);
		case EasingType::ElasticIn: return ElasticIn(t);
		case EasingType::ElasticOut: return ElasticOut(t);
		case EasingType::ElasticInOut: return ElasticInOut(t);
		}
		return Linear(t);
	}
}

inline float EaseLerp(float start, float end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}

inline Vector2 EaseLerp(const Vector2& start, const Vector2& end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}

inline Vector3 EaseLerp(const Vector3& start, const Vector3& end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}

inline Vector4 EaseLerp(const Vector4& start, const Vector4& end, float t, EasingType type) {
	return Lerp(start, end, Easing::Evaluate(type, t));
}

