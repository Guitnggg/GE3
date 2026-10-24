#include "engine/scene/SceneManager.h"

#include "engine/scene/AbstractSceneFactory.h"
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

	// SceneContextへ自身を設定し、各シーンから遅延切り替えを要求できるようにする。
	// SceneManagerが所有者であり、context内のポインターは非所有参照となる。
	context_ = context;
	context_.sceneManager = this;

	// 初期化が成功してからスタックへ所有権を移し、半端なシーンを残さない。
	initialScene->Initialize(context_);
	stack_.push_back({std::move(initialScene), {false, false}});
	initialized_ = true;
}

void SceneManager::SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory) {
	if (initialized_) {
		throw std::logic_error("Scene factory must be set before SceneManager initialization.");
	}

	if (!sceneFactory) {
		throw std::invalid_argument("Scene factory must not be null.");
	}

	// FactoryもSceneManagerが所有し、生成機能の寿命を実行期間と一致させる。
	sceneFactory_ = std::move(sceneFactory);
}

void SceneManager::Initialize(const SceneContext &context, std::string_view initialSceneId) {
	if (!sceneFactory_) {
		throw std::logic_error("Scene factory must be set before identifier based initialization.");
	}

	// ゲーム側のIDを具象シーンへ変換し、通常の初期化経路へ合流させる。
	Initialize(context, sceneFactory_->CreateScene(initialSceneId));
}

void SceneManager::Update() {
	EnsureInitialized();
	for (size_t index = FindFirstUpdateScene(); index < stack_.size(); ++index) {
		stack_[index].scene->Update();
	}
}

void SceneManager::BeginFrame() {
	EnsureInitialized();
	// UpdateやFixedUpdateの途中でシーンを破棄しないよう、操作はフレーム境界だけで適用する。
	ApplyPendingOperations();
}

void SceneManager::FixedUpdate() {
	EnsureInitialized();
	for (size_t index = FindFirstUpdateScene(); index < stack_.size(); ++index) {
		stack_[index].scene->FixedUpdate();
	}
}

void SceneManager::DrawWorld() {
	EnsureInitialized();

	for (size_t index = FindFirstDrawScene(); index < stack_.size(); ++index) {
		stack_[index].scene->DrawWorld();
	}
}

void SceneManager::DrawUI() {
	EnsureInitialized();

	for (size_t index = FindFirstDrawScene(); index < stack_.size(); ++index) {
		stack_[index].scene->DrawUI();
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

void SceneManager::ChangeScene(std::string_view nextSceneId) {
	EnsureInitialized();

	if (!sceneFactory_) {
		throw std::logic_error("Scene factory is not set.");
	}

	// ここでは生成と予約だけを行い、現在シーンの破棄はBeginFrameまで遅延する。
	ChangeScene(sceneFactory_->CreateScene(nextSceneId));
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
	// 新シーンの構築成功を先に確認し、失敗時は現在のスタックをそのまま実行可能に保つ。
	scene->Initialize(context_);
	for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
		it->scene->Finalize();
	}
	stack_.clear();
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
	// 未適用の操作もシーンを所有しているため、最初にすべて破棄する。
	pendingOperations_.clear();

	// 上に積まれたシーンから終了し、依存関係を積み上げと逆順に解消する。
	for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
		it->scene->Finalize();
	}

	stack_.clear();
	context_ = {};
	sceneFactory_.reset();
	initialized_ = false;
}
