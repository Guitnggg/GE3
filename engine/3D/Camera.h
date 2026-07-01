#pragma once

#include "engine/base/Mymath.h"

class Camera {
public:
	void Update();

	void SetRotate(const Vector3& rotate);
	void SetTranslate(const Vector3& translate);
	void SetFovY(float fovY);
	void SetAspectRatio(float aspectRatio);
	void SetNearClip(float nearClip);
	void SetFarClip(float farClip);

	const Matrix4x4& GetWorldMatrix() const;
	const Matrix4x4& GetViewMatrix() const;
	const Matrix4x4& GetProjectionMatrix() const;
	const Matrix4x4& GetViewProjectionMatrix() const;
	const Vector3& GetRotate() const;
	const Vector3& GetTranslate() const;

private:
	Transform transform_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.5f} };
	Matrix4x4 worldMatrix_ = MakeIdentity4x4();
	Matrix4x4 viewMatrix_ = MakeIdentity4x4();
	Matrix4x4 projectionMatrix_ = MakeIdentity4x4();
	Matrix4x4 viewProjectionMatrix_ = MakeIdentity4x4();
	float fovY_ = 0.45f;
	float aspectRatio_ = 1280.0f / 720.0f;
	float nearClip_ = 0.1f;
	float farClip_ = 100.0f;
};
