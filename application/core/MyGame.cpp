#include "application/core/MyGame.h"

MyGame::MyGame() = default;
MyGame::~MyGame() {
	Finalize();
}

void MyGame::Initialize() {
	Engine::Initialize();
}

void MyGame::Update() {
	Engine::Update();
}

void MyGame::FixedUpdate() {}

void MyGame::Draw() {
	BeginDraw();
	EndDraw();
}

void MyGame::Finalize() {
	Engine::Finalize();
}
