#include "engine/scene/SceneManager.h"

#include "engine/scene/IScene.h"

#include <stdexcept>

SceneManager::~SceneManager() { Finalize(); }

void SceneManager::Initialize(const SceneContext& context, std::unique_ptr<IScene> initialScene) {
	if (initialized_ || !initialScene) {
		throw std::logic_error("SceneManager initialization state or initial scene is invalid.");
	}
	context_ = context;
	currentScene_ = std::move(initialScene);
	currentScene_->Initialize(context_);
	initialized_ = true;
}

void SceneManager::Update() {
	ApplyPendingScene();
	if (currentScene_) { currentScene_->Update(); }
}

void SceneManager::FixedUpdate() {
	if (currentScene_) { currentScene_->FixedUpdate(); }
}

void SceneManager::Draw() {
	if (currentScene_) { currentScene_->Draw(); }
}

void SceneManager::ChangeScene(std::unique_ptr<IScene> nextScene) {
	if (!nextScene) { throw std::invalid_argument("Next scene must not be null."); }
	pendingScene_ = std::move(nextScene);
}

void SceneManager::ApplyPendingScene() {
	if (!pendingScene_) { return; }
	if (currentScene_) { currentScene_->Finalize(); }
	currentScene_ = std::move(pendingScene_);
	currentScene_->Initialize(context_);
}

void SceneManager::Finalize() {
	pendingScene_.reset();
	if (currentScene_) { currentScene_->Finalize(); }
	currentScene_.reset();
	context_ = {};
	initialized_ = false;
}
