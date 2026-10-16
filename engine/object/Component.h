#pragma once

class GameObject;

/// <summary>
/// GameObjectへ機能を追加するための基底クラス。
/// 派生クラスは必要なOn系フックだけをオーバーライドする。
/// </summary>
class Component {
  public:
	/// <summary>
	/// 所有者を持たない無効化されていないComponentを生成する。
	/// </summary>
	Component() = default;

	/// <summary>
	/// 派生Componentを基底ポインターから安全に破棄する。
	/// </summary>
	virtual ~Component() = default;

	/// <summary>
	/// Componentは1つのGameObjectに所属するため、コピーを禁止する。
	/// </summary>
	Component(const Component &) = delete;

	/// <summary>
	/// Componentは1つのGameObjectに所属するため、コピー代入を禁止する。
	/// </summary>
	Component &operator=(const Component &) = delete;

	/// <summary>
	/// このComponentを所有しているGameObjectを取得する。
	/// </summary>
	/// <returns>所有元のGameObjectへの参照</returns>
	[[nodiscard]] GameObject &GetGameObject() const;

	/// <summary>
	/// 更新と描画のコールバックが有効か取得する。
	/// </summary>
	/// <returns>コールバックが実行対象の場合はtrue</returns>
	[[nodiscard]] bool IsEnabled() const {
		return enabled_;
	}

	/// <summary>
	/// 更新と描画のコールバックを実行するか設定する。
	/// </summary>
	/// <param name="enabled">実行対象にする場合はtrue</param>
	void SetEnabled(bool enabled) {
		enabled_ = enabled;
	}

  protected:
	/// <summary>
	/// GameObjectへ追加された直後に一度だけ呼ばれる。
	/// </summary>
	virtual void OnAttach() {}

	/// <summary>
	/// 有効な間、GameObjectの通常更新から毎フレーム呼ばれる。
	/// </summary>
	virtual void OnUpdate() {}

	/// <summary>
	/// 有効な間、GameObjectの固定間隔更新から呼ばれる。
	/// </summary>
	virtual void OnFixedUpdate() {}

	/// <summary>
	/// 有効な間、GameObjectの描画処理から呼ばれる。
	/// </summary>
	virtual void OnDraw() {}

	/// <summary>
	/// GameObjectから取り除かれる直前に一度だけ呼ばれる。
	/// </summary>
	virtual void OnDetach() {}

  private:
	friend class GameObject;      // 所有者設定とライフサイクル呼び出しをGameObjectへ許可する
	GameObject *owner_ = nullptr; // このComponentを所有するGameObject
	bool enabled_ = true;         // 更新と描画のコールバックを実行するか
};
