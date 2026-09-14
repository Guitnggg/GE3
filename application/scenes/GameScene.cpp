#include "application/scenes/GameScene.h"

#include "engine/2D/Sprite.h"
#include "engine/3D/camera/Camera.h"
#include "engine/3D/model/MeshGenerator.h"
#include "engine/3D/model/ModelManager.h"
#include "engine/3D/object/Object3d.h"
#include "engine/3D/object/Object3dCommon.h"
#include "engine/core/WinApp.h"
#include "engine/core/timing/Time.h"
#include "engine/graphics/debug/ImGuiManager.h"
#include "engine/graphics/resource/TextureManager.h"
#include "engine/input/Input.h"
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
constexpr float kRailSpeed = 8.0f;
constexpr float kAimSpeed = 520.0f;
constexpr float kFovY = 0.70f;
constexpr float kTargetInterval = 0.85f;
constexpr float kTargetAhead = 62.0f;
constexpr uint32_t kSphereSubdivisions = 12;
}

GameScene::~GameScene() { Finalize(); }

void GameScene::Initialize(const SceneContext& context) {
	if (initialized_) { throw std::logic_error("GameScene is already initialized."); }
	context_ = context;
	try {
		texture_ = context_.textureManager->Load("resource/monsterBall.png");
		sphereModel_ = context_.modelManager->Create(MeshGenerator::CreateSphere(kSphereSubdivisions), texture_);
		camera_ = std::make_unique<Camera>();
		camera_->SetFovY(kFovY);
		camera_->SetFarClip(120.0f);

		reticle_.push_back(CreateReticlePart(42.0f, 5.0f));
		reticle_.push_back(CreateReticlePart(5.0f, 42.0f));
		for (uint32_t i = 0; i < 24; ++i) {
			const float side = (i % 2 == 0) ? -6.5f : 6.5f;
			auto marker = CreateObject(side, -3.2f, static_cast<float>(i / 2) * 7.0f, 0.22f);
			marker->GetMaterialData()->color = {0.15f, 0.55f, 1.0f, 1.0f};
			railMarkers_.push_back(std::move(marker));
		}
		initialized_ = true;
		ResetGame();
	} catch (...) { Finalize(); throw; }
}

std::unique_ptr<Object3d> GameScene::CreateObject(float x, float y, float z, float scale) {
	auto object = std::make_unique<Object3d>();
	object->Initialize(context_.object3dCommon, context_.textureManager, sphereModel_);
	object->GetTransform().translate = {x, y, z};
	object->GetTransform().scale = {scale, scale, scale};
	return object;
}

std::unique_ptr<Sprite> GameScene::CreateReticlePart(float width, float height) {
	auto sprite = std::make_unique<Sprite>();
	sprite->Initialize(context_.spriteCommon, context_.textureManager, texture_);
	sprite->SetSize({width, height});
	sprite->SetAnchorPoint({0.5f, 0.5f});
	sprite->SetColor({1.0f, 0.2f, 0.12f, 0.9f});
	return sprite;
}

void GameScene::ResetGame() {
	targets_.clear();
	cameraZ_ = -10.5f;
	aim_ = {WinApp::kClientWidth * 0.5f, WinApp::kClientHeight * 0.5f};
	score_ = 0; lives_ = 3; spawnSequence_ = 0;
	targetSpawnTimer_ = 0.2f; shotFlashTimer_ = 0.0f; gameOver_ = false;
	for (uint32_t i = 0; i < railMarkers_.size(); ++i) {
		railMarkers_[i]->GetTransform().translate.z = static_cast<float>(i / 2) * 7.0f;
	}
}

void GameScene::SpawnTarget() {
	constexpr float xPositions[] = {-4.2f, -2.1f, 0.0f, 2.1f, 4.2f};
	constexpr float yPositions[] = {-1.6f, 0.0f, 1.7f, -0.7f};
	const uint32_t xIndex = (spawnSequence_ * 3u + spawnSequence_ / 2u) % 5u;
	const uint32_t yIndex = (spawnSequence_ * 5u + 1u) % 4u;
	const float scale = 0.8f + static_cast<float>(spawnSequence_ % 3u) * 0.14f;
	Target target{};
	target.object = CreateObject(xPositions[xIndex], yPositions[yIndex], cameraZ_ + kTargetAhead, scale);
	target.object->GetMaterialData()->color = {1.0f, 0.38f, 0.3f, 1.0f};
	target.radius = scale;
	targets_.push_back(std::move(target));
	++spawnSequence_;
}

void GameScene::Shoot() {
	const float normalizedX = aim_.x / (WinApp::kClientWidth * 0.5f) - 1.0f;
	const float normalizedY = 1.0f - aim_.y / (WinApp::kClientHeight * 0.5f);
	const float tanHalfFov = std::tan(kFovY * 0.5f);
	const float aspect = static_cast<float>(WinApp::kClientWidth) / WinApp::kClientHeight;
	const Vector3 direction = Normalize({normalizedX * aspect * tanHalfFov, normalizedY * tanHalfFov, 1.0f});
	const Vector3 origin{0.0f, 0.0f, cameraZ_};
	float closest = std::numeric_limits<float>::max();
	size_t hitIndex = targets_.size();
	for (size_t i = 0; i < targets_.size(); ++i) {
		float distance = 0.0f;
		if (RayHitsSphere(origin, direction, targets_[i].object->GetTransform().translate,
			targets_[i].radius, distance) && distance < closest) {
			closest = distance;
			hitIndex = i;
		}
	}
	if (hitIndex != targets_.size()) {
		targets_.erase(targets_.begin() + hitIndex);
		++score_;
	}
	shotFlashTimer_ = 0.08f;
}

bool GameScene::RayHitsSphere(const Vector3& origin, const Vector3& direction, const Vector3& center,
	float radius, float& distance) {
	const Vector3 offset{origin.x - center.x, origin.y - center.y, origin.z - center.z};
	const float b = offset.x * direction.x + offset.y * direction.y + offset.z * direction.z;
	const float c = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z - radius * radius;
	const float discriminant = b * b - c;
	if (discriminant < 0.0f) { return false; }
	distance = -b - std::sqrt(discriminant);
	return distance >= 0.0f;
}

void GameScene::UpdateRail(float deltaTime) {
	cameraZ_ += kRailSpeed * deltaTime;
	for (auto& marker : railMarkers_) {
		if (marker->GetTransform().translate.z < cameraZ_ - 2.0f) {
			marker->GetTransform().translate.z += 84.0f;
		}
	}
	for (auto it = targets_.begin(); it != targets_.end();) {
		if (it->object->GetTransform().translate.z < cameraZ_ + 0.8f) {
			if (lives_ > 0) { --lives_; }
			it = targets_.erase(it);
		} else { ++it; }
	}
	if (lives_ == 0) { gameOver_ = true; }
}

void GameScene::UpdateObjects() {
	camera_->SetTranslate({0.0f, 0.0f, cameraZ_});
	camera_->Update();
	for (auto& marker : railMarkers_) { marker->Update(*camera_); }
	for (Target& target : targets_) {
		target.object->GetTransform().rotate.y += 0.025f;
		target.object->Update(*camera_);
	}
	for (auto& part : reticle_) {
		part->GetTransform().translate = {aim_.x, aim_.y, 0.0f};
		part->SetColor(shotFlashTimer_ > 0.0f ? Vector4{1.0f, 1.0f, 0.2f, 1.0f} : Vector4{1.0f, 0.2f, 0.12f, 0.9f});
		part->Update(WinApp::kClientWidth, WinApp::kClientHeight);
	}
}

void GameScene::Update() {
	const float dt = context_.time->GetDeltaTime();
	if (!gameOver_) {
		float x = 0.0f, y = 0.0f;
		if (context_.input->PushKey(DIK_A) || context_.input->PushKey(DIK_LEFT)) { x -= 1.0f; }
		if (context_.input->PushKey(DIK_D) || context_.input->PushKey(DIK_RIGHT)) { x += 1.0f; }
		if (context_.input->PushKey(DIK_W) || context_.input->PushKey(DIK_UP)) { y -= 1.0f; }
		if (context_.input->PushKey(DIK_S) || context_.input->PushKey(DIK_DOWN)) { y += 1.0f; }
		aim_.x = std::clamp(aim_.x + x * kAimSpeed * dt, 20.0f, WinApp::kClientWidth - 20.0f);
		aim_.y = std::clamp(aim_.y + y * kAimSpeed * dt, 20.0f, WinApp::kClientHeight - 20.0f);
		if (context_.input->TriggerKey(DIK_SPACE)) { Shoot(); }
		UpdateRail(dt);
		targetSpawnTimer_ -= dt;
		if (targetSpawnTimer_ <= 0.0f) {
			SpawnTarget();
			targetSpawnTimer_ = std::max(0.38f, kTargetInterval - static_cast<float>(score_) * 0.012f);
		}
	} else if (context_.input->TriggerKey(DIK_R)) { ResetGame(); }
	shotFlashTimer_ = std::max(0.0f, shotFlashTimer_ - dt);
	UpdateObjects();

#ifdef _DEBUG
	context_.imguiManager->BeginFrame();
	ImGui::SetNextWindowPos({12.0f, 12.0f}, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.72f);
	ImGui::Begin("3D RAIL SHOOTER", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Text("SCORE  %u", score_);
	ImGui::Text("LIVES  %u", lives_);
	ImGui::TextUnformatted("AIM: WASD / Arrow Keys    FIRE: Space");
	if (gameOver_) { ImGui::Separator(); ImGui::TextColored({1.0f, 0.25f, 0.2f, 1.0f}, "GAME OVER"); ImGui::TextUnformatted("Press R to restart"); }
	ImGui::End();
	context_.imguiManager->EndFrame();
#endif
}

void GameScene::FixedUpdate() {}

void GameScene::Draw() {
	context_.object3dCommon->CommonDrawSetting();
	for (const auto& marker : railMarkers_) { marker->Draw(); }
	for (const Target& target : targets_) { target.object->Draw(); }
	for (const auto& part : reticle_) { part->Draw(); }
}

void GameScene::Finalize() {
	initialized_ = false;
	targets_.clear(); railMarkers_.clear(); reticle_.clear(); camera_.reset(); sphereModel_.reset();
	texture_ = 0; context_ = {};
}
