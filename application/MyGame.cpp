#include "application/MyGame.h"

#include "application/scenes/GameScene.h"
#include "engine/scene/SceneManager.h"

#include <stdexcept>

MyGame::MyGame() = default;
MyGame::~MyGame() { Finalize(); }

void MyGame::Initialize() {
	if (initialized_ || sceneManager_) {
		throw std::logic_error("MyGame is already initialized or partially initialized.");
	}

	try {
		Framework::Initialize();
		SceneContext context{};
		context.audio = audio_.get();
		context.input = input_.get();
		context.textureManager = textureManager_.get();
		context.modelManager = modelManager_.get();
		context.spriteCommon = spriteCommon_.get();
		context.object3dCommon = object3dCommon_.get();
		context.time = time_.get();
		context.frameRateController = frameRateController_.get();
#ifdef _DEBUG
		context.imguiManager = imguiManager_.get();
#endif

		sceneManager_ = std::make_unique<SceneManager>();
		sceneManager_->Initialize(context, std::make_unique<GameScene>());
		initialized_ = true;
	} catch (...) {
		Finalize();
		throw;
	}
}

void MyGame::Update() {
	Framework::Update();
	sceneManager_->Update();
}

void MyGame::FixedUpdate() {
	if (sceneManager_) { sceneManager_->FixedUpdate(); }
}

void MyGame::Draw() {
	BeginDraw();
	sceneManager_->Draw();
	EndDraw();
}

void MyGame::Finalize() {
	initialized_ = false;
	if (sceneManager_) { sceneManager_->Finalize(); }
	sceneManager_.reset();
	Framework::Finalize();
}
