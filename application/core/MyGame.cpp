#include "application/core/MyGame.h"

MyGame::MyGame() = default;
MyGame::~MyGame() {
	Finalize();
}

void MyGame::Initialize() {
	Framework::Initialize();
}

void MyGame::Update() {
	Framework::Update();
}

void MyGame::FixedUpdate() {}

void MyGame::Draw() {
	BeginDraw();
	EndDraw();
}

void MyGame::Finalize() {
	Framework::Finalize();
}
