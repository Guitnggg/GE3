#include "Camera.h"
#include <numbers>
#include <stdexcept>

void Camera::Update() {
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

void Camera::SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }

void Camera::SetTranslate(const Vector3& translate) { transform_.translate = translate; }

void Camera::SetFovY(float fovY) {
	if (fovY <= 0.0f || fovY >= std::numbers::pi_v<float>) { throw std::invalid_argument("Camera FOV must be between 0 and pi."); }
	fovY_ = fovY;
}

void Camera::SetAspectRatio(float aspectRatio) {
	if (aspectRatio <= 0.0f) { throw std::invalid_argument("Camera aspect ratio must be positive."); }
	aspectRatio_ = aspectRatio;
}

void Camera::SetNearClip(float nearClip) {
	if (nearClip <= 0.0f || nearClip >= farClip_) { throw std::invalid_argument("Camera near clip must be positive and less than the far clip."); }
	nearClip_ = nearClip;
}

void Camera::SetFarClip(float farClip) {
	if (farClip <= nearClip_) { throw std::invalid_argument("Camera far clip must be greater than the near clip."); }
	farClip_ = farClip;
}

const Matrix4x4& Camera::GetWorldMatrix() const { return worldMatrix_; }

const Matrix4x4& Camera::GetViewMatrix() const { return viewMatrix_; }

const Matrix4x4& Camera::GetProjectionMatrix() const { return projectionMatrix_; }

const Matrix4x4& Camera::GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

const Vector3& Camera::GetRotate() const { return transform_.rotate; }

const Vector3& Camera::GetTranslate() const { return transform_.translate; }
