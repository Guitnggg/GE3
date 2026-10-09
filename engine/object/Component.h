#pragma once

class GameObject;

/// <summary>
/// GameObjectへ機能を追加するための基底クラス。
/// 派生クラスは必要なOn系フックだけをオーバーライドする。
/// </summary>
class Component {
  public:
	Component() = default;
	virtual ~Component() = default;
	Component(const Component &) = delete;
	Component &operator=(const Component &) = delete;

	[[nodiscard]] GameObject &GetGameObject() const;
	[[nodiscard]] bool IsEnabled() const {
		return enabled_;
	}
	void SetEnabled(bool enabled) {
		enabled_ = enabled;
	}

  protected:
	virtual void OnAttach() {}
	virtual void OnUpdate() {}
	virtual void OnFixedUpdate() {}
	virtual void OnDraw() {}
	virtual void OnDetach() {}

  private:
	friend class GameObject;
	GameObject *owner_ = nullptr;
	bool enabled_ = true;
};
