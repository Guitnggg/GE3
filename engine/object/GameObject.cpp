#include "engine/object/GameObject.h"

#include <stdexcept>

GameObject::GameObject(std::string name) : name_(std::move(name)) {
	if (name_.empty()) {
		throw std::invalid_argument("GameObject name must not be empty.");
	}
}

GameObject::~GameObject() {
	try {
		ClearComponents();
	} catch (...) {
		// デストラクタから例外を送出せず、残りのComponentはunique_ptrに解放させる
	}
}

void GameObject::Update() {
	Dispatch([](Component &component) { component.OnUpdate(); });
}

void GameObject::FixedUpdate() {
	Dispatch([](Component &component) { component.OnFixedUpdate(); });
}

void GameObject::Draw() {
	Dispatch([](Component &component) { component.OnDraw(); });
}

void GameObject::ClearComponents() {
	EnsureComponentsMutable();
	for (auto it = components_.rbegin(); it != components_.rend(); ++it) {
		(*it)->OnDetach();
		(*it)->owner_ = nullptr;
	}
	components_.clear();
}

void GameObject::SetName(std::string name) {
	if (name.empty()) {
		throw std::invalid_argument("GameObject name must not be empty.");
	}
	name_ = std::move(name);
}

void GameObject::EnsureComponentsMutable() const {
	if (dispatching_) {
		throw std::logic_error("Components cannot be added or removed while GameObject is dispatching events.");
	}
}
