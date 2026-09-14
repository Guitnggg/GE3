#include "application/characters/Player.h"

#include "engine/core/WinApp.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/input/Input.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr float kFovY = 0.70f;
}

void Player::Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, Input* input, uint32_t texture) {
	// プレイヤーが利用する機能は所有せず、Frameworkより長く保持しない参照として保存する
	if (!spriteCommon || !textureManager || !input) { throw std::invalid_argument("Player requires initialized engine services."); }
	spriteCommon_ = spriteCommon;
	textureManager_ = textureManager;
	input_ = input;
	camera_.SetFovY(kFovY);
	camera_.SetFarClip(120.0f);
	// 横線と縦線を重ねて照準を構成する
	reticle_.push_back(CreateReticlePart(42.0f, 5.0f, texture));
	reticle_.push_back(CreateReticlePart(5.0f, 42.0f, texture));
	Reset();
}

std::unique_ptr<Sprite> Player::CreateReticlePart(float width, float height, uint32_t texture) {
	// 中央基準にすることで、縦線と横線を同じ座標へ正確に重ねられる
	auto sprite = std::make_unique<Sprite>();
	sprite->Initialize(spriteCommon_, textureManager_, texture);
	sprite->SetSize({width, height});
	sprite->SetAnchorPoint({0.5f, 0.5f});
	return sprite;
}

void Player::Reset(uint32_t startingLives) {
	// ゲーム再開時に、進行位置と画面中央の照準を初期状態へ戻す
	cameraZ_ = -10.5f;
	aim_ = {WinApp::kClientWidth * 0.5f, WinApp::kClientHeight * 0.5f};
	shotFlashTimer_ = 0.0f;
	lives_ = startingLives;
	camera_.SetTranslate({0.0f, 0.0f, cameraZ_});
	camera_.Update();
	UpdateReticle();
}

bool Player::Update(float deltaTime, float railSpeed, float aimSpeed) {
	// WASDと矢印キーのどちらでも照準を操作できるよう入力方向を作る
	float x = 0.0f;
	float y = 0.0f;
	if (input_->PushKey(DIK_A) || input_->PushKey(DIK_LEFT)) { x -= 1.0f; }
	if (input_->PushKey(DIK_D) || input_->PushKey(DIK_RIGHT)) { x += 1.0f; }
	if (input_->PushKey(DIK_W) || input_->PushKey(DIK_UP)) { y -= 1.0f; }
	if (input_->PushKey(DIK_S) || input_->PushKey(DIK_DOWN)) { y += 1.0f; }
	// 照準を画面内に制限し、フレーム時間に依存しない速度でカメラを前進させる
	aim_.x = std::clamp(aim_.x + x * aimSpeed * deltaTime, 20.0f, WinApp::kClientWidth - 20.0f);
	aim_.y = std::clamp(aim_.y + y * aimSpeed * deltaTime, 20.0f, WinApp::kClientHeight - 20.0f);
	cameraZ_ += railSpeed * deltaTime;
	// 押した瞬間だけ射撃し、短時間だけ照準色を変えて反応を示す
	const bool fired = input_->TriggerKey(DIK_SPACE);
	if (fired) { shotFlashTimer_ = 0.08f; }
	shotFlashTimer_ = std::max(0.0f, shotFlashTimer_ - deltaTime);
	camera_.SetTranslate({0.0f, 0.0f, cameraZ_});
	camera_.Update();
	UpdateReticle();
	return fired;
}

void Player::UpdateReticle() {
	// 2本のスプライトへ同じ位置と射撃中の色を反映する
	for (auto& part : reticle_) {
		part->GetTransform().translate = {aim_.x, aim_.y, 0.0f};
		part->SetColor(shotFlashTimer_ > 0.0f ? Vector4{1.0f, 1.0f, 0.2f, 1.0f} : Vector4{1.0f, 0.2f, 0.12f, 0.9f});
		part->Update(WinApp::kClientWidth, WinApp::kClientHeight);
	}
}

// カメラはX・Y方向へ移動しないため、レール上のZ座標だけを射線始点へ反映する
Vector3 Player::GetShotOrigin() const { return {0.0f, 0.0f, cameraZ_}; }

Vector3 Player::GetShotDirection() const {
	// 画面座標を透視投影の正規化座標へ直し、カメラ前方へ向かう射線を作る
	const float normalizedX = aim_.x / (WinApp::kClientWidth * 0.5f) - 1.0f;
	const float normalizedY = 1.0f - aim_.y / (WinApp::kClientHeight * 0.5f);
	const float tanHalfFov = std::tan(kFovY * 0.5f);
	const float aspect = static_cast<float>(WinApp::kClientWidth) / WinApp::kClientHeight;
	return Normalize({normalizedX * aspect * tanHalfFov, normalizedY * tanHalfFov, 1.0f});
}

// unsigned整数のアンダーフローを防ぐため、0より大きい場合だけ減算する
void Player::Damage() { if (lives_ > 0) { --lives_; } }

// 2Dパイプラインへの切り替えはSprite::Drawが行うため、各部品を順番に描画する
void Player::DrawReticle() const { for (const auto& part : reticle_) { part->Draw(); } }
