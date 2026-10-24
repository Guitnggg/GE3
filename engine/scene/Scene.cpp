#include "engine/scene/Scene.h"

#include <algorithm>
#include <stdexcept>

void Scene::Initialize(const SceneContext &context) {
	if (initialized_) {
		throw std::logic_error("Scene is already initialized.");
	}
	context_ = context;
	initialized_ = true;
	try {
		OnInitialize();
	} catch (...) {
		initialized_ = false;
		context_ = {};
		throw;
	}
}

void Scene::Update() {
	if (!initialized_) {
		throw std::logic_error("Scene is not initialized.");
	}
	// シーン全体の状態を先に更新し、その確定値を各Componentへ伝える。
	OnUpdate();
	for (const auto &gameObject : gameObjects_) {
		gameObject->Update();
	}
	FlushDestroyedObjects();
}

void Scene::FixedUpdate() {
	if (!initialized_) {
		throw std::logic_error("Scene is not initialized.");
	}
	// Colliderを同期するComponentをすべて進めた後、EngineがCollisionWorldを一度だけ評価する。
	OnFixedUpdate();
	for (const auto &gameObject : gameObjects_) {
		gameObject->FixedUpdate();
	}
	// 固定更新が複数回走る場合も、次のステップへ破棄予定オブジェクトを残さない。
	FlushDestroyedObjects();
}

void Scene::DrawWorld() {
	if (!initialized_) {
		throw std::logic_error("Scene is not initialized.");
	}

	// GameObjectは既存の描画規約を維持し、ワールド描画として扱う。
	// UI専用の要素は派生シーンのOnDrawUIで明示的に描画する。
	OnDrawWorld();

	for (const auto &gameObject : gameObjects_) {
		gameObject->Draw();
	}

	FlushDestroyedObjects();
}

void Scene::DrawUI() {
	if (!initialized_) {
		throw std::logic_error("Scene is not initialized.");
	}

	// UIはワールド描画完了後に呼ばれ、将来はポストエフェクト合成後の描画先を使用する。
	OnDrawUI();

	FlushDestroyedObjects();
}

void Scene::Finalize() {
	if (!initialized_) {
		return;
	}
	// 派生シーンがGameObjectを参照できるうちに終了処理を行い、その後で所有物を破棄する。
	OnFinalize();
	pendingDestroy_.clear();
	gameObjects_.clear();
	context_ = {};
	initialized_ = false;
}

GameObject &Scene::CreateGameObject(std::string name) {
	if (!initialized_) {
		throw std::logic_error("GameObjects can only be created by an initialized scene.");
	}
	auto gameObject = std::make_unique<GameObject>(std::move(name));
	GameObject &result = *gameObject;
	gameObjects_.push_back(std::move(gameObject));
	return result;
}

void Scene::DestroyGameObject(GameObject &gameObject) {
	const auto owned = std::find_if(gameObjects_.begin(), gameObjects_.end(),
	                                [&gameObject](const auto &entry) { return entry.get() == &gameObject; });
	if (owned == gameObjects_.end()) {
		throw std::invalid_argument("GameObject is not owned by this scene.");
	}
	if (std::find(pendingDestroy_.begin(), pendingDestroy_.end(), &gameObject) == pendingDestroy_.end()) {
		// 実体の削除までは遅延するが、要求後のUpdate・FixedUpdate・Drawは即座に停止する。
		gameObject.pendingDestroy_ = true;
		pendingDestroy_.push_back(&gameObject);
	}
}

const SceneContext &Scene::GetContext() const {
	if (!initialized_) {
		throw std::logic_error("Scene is not initialized.");
	}
	return context_;
}

void Scene::FlushDestroyedObjects() {
	for (GameObject *target : pendingDestroy_) {
		std::erase_if(gameObjects_, [target](const auto &entry) { return entry.get() == target; });
	}
	pendingDestroy_.clear();
}
