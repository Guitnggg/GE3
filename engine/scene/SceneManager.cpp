#include "engine/scene/SceneManager.h"

#include "engine/scene/IScene.h"

#include <stdexcept>

SceneManager::~SceneManager() { Finalize(); }

void SceneManager::Initialize(const SceneContext& context, std::unique_ptr<IScene> initialScene) {
	// 多重初期化と空の初期シーンを拒否する
	if (initialized_ || !initialScene) {
		throw std::logic_error("SceneManager initialization state or initial scene is invalid.");
	}
	// 共通機能を保持し、所有権を受け取った最初のシーンを開始する
	context_ = context;
	currentScene_ = std::move(initialScene);
	currentScene_->Initialize(context_);
	initialized_ = true;
}

void SceneManager::Update() {
	// 更新中のシーン破棄を避けるため、予約された切り替えをフレーム先頭で適用する
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
	// nullptrでは現在のシーンを意図せず失わないようにする
	if (!nextScene) { throw std::invalid_argument("Next scene must not be null."); }
	// 実際の切り替えは次のUpdate開始時まで遅延する
	pendingScene_ = std::move(nextScene);
}

void SceneManager::ApplyPendingScene() {
	if (!pendingScene_) { return; }

	// 現在のシーンを終了してから、同じ共通機能で次のシーンを開始する
	if (currentScene_) { currentScene_->Finalize(); }
	currentScene_ = std::move(pendingScene_);
	currentScene_->Initialize(context_);
}

void SceneManager::Finalize() {
	// 未開始の予約シーンを破棄し、実行中のシーンだけ終了処理を呼ぶ
	pendingScene_.reset();
	if (currentScene_) { currentScene_->Finalize(); }
	currentScene_.reset();
	context_ = {};
	initialized_ = false;
}
