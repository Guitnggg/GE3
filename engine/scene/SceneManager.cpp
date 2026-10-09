#include "engine/scene/SceneManager.h"

#include "engine/scene/IScene.h"

#include <stdexcept>

SceneManager::~SceneManager() {
	Finalize();
}

void SceneManager::Initialize(const SceneContext &context, std::unique_ptr<IScene> initialScene) {
	// 多重初期化と空の初期シーンを拒否する
	if (initialized_ || !initialScene) {
		throw std::logic_error("SceneManager initialization state or initial scene is invalid.");
	}
	// 共通機能を保持し、所有権を受け取った最初のシーンを開始する
	context_ = context;
	initialScene->Initialize(context_);
	stack_.push_back({std::move(initialScene), {false, false}});
	initialized_ = true;
}

void SceneManager::Update() {
	EnsureInitialized();
	// コールバック中のシーン破棄を避けるため、予約操作はフレーム先頭で適用する
	ApplyPendingOperations();
	for (size_t index = FindFirstUpdateScene(); index < stack_.size(); ++index) {
		stack_[index].scene->Update();
	}
}

void SceneManager::FixedUpdate() {
	EnsureInitialized();
	for (size_t index = FindFirstUpdateScene(); index < stack_.size(); ++index) {
		stack_[index].scene->FixedUpdate();
	}
}

void SceneManager::Draw() {
	EnsureInitialized();
	for (size_t index = FindFirstDrawScene(); index < stack_.size(); ++index) {
		stack_[index].scene->Draw();
	}
}

void SceneManager::ChangeScene(std::unique_ptr<IScene> nextScene) {
	// nullptrでは現在のシーンを意図せず失わないようにする
	if (!nextScene) {
		throw std::invalid_argument("Next scene must not be null.");
	}
	EnsureInitialized();
	pendingOperations_.push_back({OperationType::Replace, std::move(nextScene), {false, false}});
}

void SceneManager::PushScene(std::unique_ptr<IScene> scene, LayerOptions options) {
	if (!scene) {
		throw std::invalid_argument("Scene to push must not be null.");
	}
	EnsureInitialized();
	pendingOperations_.push_back({OperationType::Push, std::move(scene), options});
}

void SceneManager::PopScene() {
	EnsureInitialized();
	pendingOperations_.push_back({OperationType::Pop, nullptr, {}});
}

IScene *SceneManager::GetTopScene() {
	return stack_.empty() ? nullptr : stack_.back().scene.get();
}

const IScene *SceneManager::GetTopScene() const {
	return stack_.empty() ? nullptr : stack_.back().scene.get();
}

void SceneManager::ApplyPendingOperations() {
	while (!pendingOperations_.empty()) {
		PendingOperation operation = std::move(pendingOperations_.front());
		pendingOperations_.pop_front();
		switch (operation.type) {
		case OperationType::Replace:
			ApplyReplace(std::move(operation.scene));
			break;
		case OperationType::Push:
			ApplyPush(std::move(operation.scene), operation.options);
			break;
		case OperationType::Pop:
			ApplyPop();
			break;
		}
	}
}

void SceneManager::ApplyReplace(std::unique_ptr<IScene> scene) {
	for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
		it->scene->Finalize();
	}
	stack_.clear();
	scene->Initialize(context_);
	stack_.push_back({std::move(scene), {false, false}});
}

void SceneManager::ApplyPush(std::unique_ptr<IScene> scene, LayerOptions options) {
	if (!stack_.empty()) {
		stack_.back().scene->OnPause();
	}
	try {
		scene->Initialize(context_);
		stack_.push_back({std::move(scene), options});
	} catch (...) {
		if (!stack_.empty()) {
			stack_.back().scene->OnResume();
		}
		throw;
	}
}

void SceneManager::ApplyPop() {
	// 最後のシーンは維持し、空のSceneManagerが実行される状態を防ぐ
	if (stack_.size() <= 1) {
		return;
	}
	stack_.back().scene->Finalize();
	stack_.pop_back();
	stack_.back().scene->OnResume();
}

size_t SceneManager::FindFirstUpdateScene() const {
	size_t index = stack_.size() - 1;
	while (index > 0 && stack_[index].options.updateBelow) {
		--index;
	}
	return index;
}

size_t SceneManager::FindFirstDrawScene() const {
	size_t index = stack_.size() - 1;
	while (index > 0 && stack_[index].options.drawBelow) {
		--index;
	}
	return index;
}

void SceneManager::EnsureInitialized() const {
	if (!initialized_ || stack_.empty()) {
		throw std::logic_error("SceneManager is not initialized.");
	}
}

void SceneManager::Finalize() {
	pendingOperations_.clear();
	for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
		it->scene->Finalize();
	}
	stack_.clear();
	context_ = {};
	initialized_ = false;
}
