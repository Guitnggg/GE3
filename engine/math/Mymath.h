#pragma once

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

