#pragma once

#include "engine/math/MathTypes.h"
#include "engine/object/Component.h"

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/// <summary>
/// シーン内の1オブジェクトと、それを構成するComponent群を所有する。
/// </summary>
class GameObject final {
  public:
	explicit GameObject(std::string name = "GameObject");
	~GameObject();
	GameObject(const GameObject &) = delete;
	GameObject &operator=(const GameObject &) = delete;
	GameObject(GameObject &&) = delete;
	GameObject &operator=(GameObject &&) = delete;

	template <typename T, typename... Args>
		requires std::derived_from<T, Component>
	T &AddComponent(Args &&...args) {
		EnsureComponentsMutable();
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		T &result = *component;
		component->owner_ = this;
		components_.push_back(std::move(component));
		try {
			result.OnAttach();
		} catch (...) {
			components_.pop_back();
			throw;
		}
		return result;
	}

	template <typename T>
		requires std::derived_from<T, Component>
	[[nodiscard]] T *GetComponent() {
		for (const auto &component : components_) {
			if (auto *result = dynamic_cast<T *>(component.get())) {
				return result;
			}
		}
		return nullptr;
	}

	template <typename T>
		requires std::derived_from<T, Component>
	[[nodiscard]] const T *GetComponent() const {
		for (const auto &component : components_) {
			if (const auto *result = dynamic_cast<const T *>(component.get())) {
				return result;
			}
		}
		return nullptr;
	}

	template <typename T>
		requires std::derived_from<T, Component>
	bool RemoveComponent() {
		EnsureComponentsMutable();
		for (auto it = components_.begin(); it != components_.end(); ++it) {
			if (dynamic_cast<T *>(it->get()) != nullptr) {
				(*it)->OnDetach();
				(*it)->owner_ = nullptr;
				components_.erase(it);
				return true;
			}
		}
		return false;
	}

	void Update();
	void FixedUpdate();
	void Draw();
	void ClearComponents();

	[[nodiscard]] const std::string &GetName() const {
		return name_;
	}
	void SetName(std::string name);
	[[nodiscard]] bool IsActive() const {
		return active_;
	}
	void SetActive(bool active) {
		active_ = active;
	}
	[[nodiscard]] Transform &GetTransform() {
		return transform_;
	}
	[[nodiscard]] const Transform &GetTransform() const {
		return transform_;
	}
	[[nodiscard]] size_t GetComponentCount() const {
		return components_.size();
	}

  private:
	template <typename Callback>
	void Dispatch(Callback &&callback) {
		if (!active_) {
			return;
		}
		dispatching_ = true;
		try {
			for (const auto &component : components_) {
				if (component->enabled_) {
					callback(*component);
				}
			}
		} catch (...) {
			dispatching_ = false;
			throw;
		}
		dispatching_ = false;
	}

	void EnsureComponentsMutable() const;

	std::string name_;
	Transform transform_{{1.0f, 1.0f, 1.0f}, {}, {}};
	std::vector<std::unique_ptr<Component>> components_;
	bool active_ = true;
	bool dispatching_ = false;
};
