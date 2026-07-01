#include "Camera.h"

void Camera::Update() {
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

void Camera::SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }

void Camera::SetTranslate(const Vector3& translate) { transform_.translate = translate; }

void Camera::SetFovY(float fovY) { fovY_ = fovY; }

void Camera::SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }

void Camera::SetNearClip(float nearClip) { nearClip_ = nearClip; }

void Camera::SetFarClip(float farClip) { farClip_ = farClip; }

const Matrix4x4& Camera::GetWorldMatrix() const { return worldMatrix_; }

const Matrix4x4& Camera::GetViewMatrix() const { return viewMatrix_; }

const Matrix4x4& Camera::GetProjectionMatrix() const { return projectionMatrix_; }

const Matrix4x4& Camera::GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

const Vector3& Camera::GetRotate() const { return transform_.rotate; }

const Vector3& Camera::GetTranslate() const { return transform_.translate; }
