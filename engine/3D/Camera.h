#pragma once

#include "engine/core/Mymath.h"

/// <summary>
/// 3D空間を映すカメラの姿勢と投影設定を管理するクラス。
/// </summary>
class Camera {
public:
	/// <summary>
	/// 現在の姿勢と投影設定から各種行列を更新する。
	/// </summary>
	void Update();

	/// <summary>
	/// カメラの回転角を設定する。
	/// </summary>
	void SetRotate(const Vector3& rotate);

	/// <summary>
	/// カメラの位置を設定する。
	/// </summary>
	void SetTranslate(const Vector3& translate);

	/// <summary>
	/// 縦方向の視野角をラジアン単位で設定する。
	/// </summary>
	void SetFovY(float fovY);
	
	/// <summary>
	/// 画面のアスペクト比を設定する。
	/// </summary>
	void SetAspectRatio(float aspectRatio);
	
	/// <summary>
	/// 描画する最短距離を設定する。
	/// </summary>
	void SetNearClip(float nearClip);
	
	/// <summary>
	/// 描画する最長距離を設定する。
	/// </summary>
	void SetFarClip(float farClip);


	/// <summary>
	/// カメラのワールド行列を取得する。
	/// </summary>
	const Matrix4x4& GetWorldMatrix() const;
	
	/// <summary>
	/// ビュー行列を取得する。
	/// </summary>
	const Matrix4x4& GetViewMatrix() const;
	
	/// <summary>
	/// 透視投影行列を取得する。
	/// </summary>
	const Matrix4x4& GetProjectionMatrix() const;
	
	/// <summary>
	/// ビュー行列と投影行列を合成した行列を取得する。
	/// </summary>
	const Matrix4x4& GetViewProjectionMatrix() const;
	
	/// <summary>
	/// 現在の回転角を取得する。
	/// </summary>
	const Vector3& GetRotate() const;
	
	/// <summary>
	/// 現在の位置を取得する。
	/// </summary>
	const Vector3& GetTranslate() const;

private:
	// カメラの座標変換
	Transform transform_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.5f} };

	// Updateで計算した行列
	Matrix4x4 worldMatrix_ = MakeIdentity4x4();
	Matrix4x4 viewMatrix_ = MakeIdentity4x4();
	Matrix4x4 projectionMatrix_ = MakeIdentity4x4();
	Matrix4x4 viewProjectionMatrix_ = MakeIdentity4x4();

	// 透視投影に使用する設定値
	float fovY_ = 0.45f;
	float aspectRatio_ = 1280.0f / 720.0f;
	float nearClip_ = 0.1f;
	float farClip_ = 100.0f;
};
