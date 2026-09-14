#include "application/characters/Player.h"

#include "engine/core/WinApp.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/input/Input.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr float kRailSpeed = 8.0f;
constexpr float kAimSpeed = 520.0f;
constexpr float kFovY = 0.70f;
}

void Player::Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, Input* input, uint32_t texture) {
	if (!spriteCommon || !textureManager || !input) { throw std::invalid_argument("Player requires initialized engine services."); }
	spriteCommon_ = spriteCommon;
	textureManager_ = textureManager;
	input_ = input;
	camera_.SetFovY(kFovY);
	camera_.SetFarClip(120.0f);
	reticle_.push_back(CreateReticlePart(42.0f, 5.0f, texture));
	reticle_.push_back(CreateReticlePart(5.0f, 42.0f, texture));
	Reset();
}

std::unique_ptr<Sprite> Player::CreateReticlePart(float width, float height, uint32_t texture) {
	auto sprite = std::make_unique<Sprite>();
	sprite->Initialize(spriteCommon_, textureManager_, texture);
	sprite->SetSize({width, height});
	sprite->SetAnchorPoint({0.5f, 0.5f});
	return sprite;
}

void Player::Reset() {
	cameraZ_ = -10.5f;
	aim_ = {WinApp::kClientWidth * 0.5f, WinApp::kClientHeight * 0.5f};
	shotFlashTimer_ = 0.0f;
	lives_ = 3;
	camera_.SetTranslate({0.0f, 0.0f, cameraZ_});
	camera_.Update();
	UpdateReticle();
}

bool Player::Update(float deltaTime) {
	float x = 0.0f;
	float y = 0.0f;
	if (input_->PushKey(DIK_A) || input_->PushKey(DIK_LEFT)) { x -= 1.0f; }
	if (input_->PushKey(DIK_D) || input_->PushKey(DIK_RIGHT)) { x += 1.0f; }
	if (input_->PushKey(DIK_W) || input_->PushKey(DIK_UP)) { y -= 1.0f; }
	if (input_->PushKey(DIK_S) || input_->PushKey(DIK_DOWN)) { y += 1.0f; }
	aim_.x = std::clamp(aim_.x + x * kAimSpeed * deltaTime, 20.0f, WinApp::kClientWidth - 20.0f);
	aim_.y = std::clamp(aim_.y + y * kAimSpeed * deltaTime, 20.0f, WinApp::kClientHeight - 20.0f);
	cameraZ_ += kRailSpeed * deltaTime;
	const bool fired = input_->TriggerKey(DIK_SPACE);
	if (fired) { shotFlashTimer_ = 0.08f; }
	shotFlashTimer_ = std::max(0.0f, shotFlashTimer_ - deltaTime);
	camera_.SetTranslate({0.0f, 0.0f, cameraZ_});
	camera_.Update();
	UpdateReticle();
	return fired;
}

void Player::UpdateReticle() {
	for (auto& part : reticle_) {
		part->GetTransform().translate = {aim_.x, aim_.y, 0.0f};
		part->SetColor(shotFlashTimer_ > 0.0f ? Vector4{1.0f, 1.0f, 0.2f, 1.0f} : Vector4{1.0f, 0.2f, 0.12f, 0.9f});
		part->Update(WinApp::kClientWidth, WinApp::kClientHeight);
	}
}

Vector3 Player::GetShotOrigin() const { return {0.0f, 0.0f, cameraZ_}; }

Vector3 Player::GetShotDirection() const {
	const float normalizedX = aim_.x / (WinApp::kClientWidth * 0.5f) - 1.0f;
	const float normalizedY = 1.0f - aim_.y / (WinApp::kClientHeight * 0.5f);
	const float tanHalfFov = std::tan(kFovY * 0.5f);
	const float aspect = static_cast<float>(WinApp::kClientWidth) / WinApp::kClientHeight;
	return Normalize({normalizedX * aspect * tanHalfFov, normalizedY * tanHalfFov, 1.0f});
}

void Player::Damage() { if (lives_ > 0) { --lives_; } }

void Player::DrawReticle() const { for (const auto& part : reticle_) { part->Draw(); } }
