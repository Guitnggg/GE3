#pragma once

#include "engine/object/Component.h"
#include "engine/object/TransformNode.h"

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/// <summary>
/// シーン内の1つのオブジェクトを表すクラス。
/// 座標情報とComponentを所有し、登録されたComponentの更新と描画をまとめて実行する。
/// </summary>
class GameObject final {
  public:
	/// <summary>
	/// 指定した名前でGameObjectを生成する。
	/// </summary>
	/// <param name="name">デバッグ表示や検索に使用する名前</param>
	explicit GameObject(std::string name = "GameObject");

	/// <summary>
	/// 所有しているすべてのComponentを切り離して破棄する。
	/// </summary>
	~GameObject();

	/// <summary>
	/// GameObjectはComponentと座標情報を単独所有するため、コピーを禁止する。
	/// </summary>
	GameObject(const GameObject &) = delete;

	/// <summary>
	/// GameObjectはComponentと座標情報を単独所有するため、コピー代入を禁止する。
	/// </summary>
	GameObject &operator=(const GameObject &) = delete;

	/// <summary>
	/// TransformNodeの親子参照を安全に保つため、ムーブを禁止する。
	/// </summary>
	GameObject(GameObject &&) = delete;

	/// <summary>
	/// TransformNodeの親子参照を安全に保つため、ムーブ代入を禁止する。
	/// </summary>
	GameObject &operator=(GameObject &&) = delete;

	/// <summary>
	/// 新しいComponentを生成して、このGameObjectへ追加する。
	/// 追加後にComponent::OnAttachを呼び出す。
	/// </summary>
	/// <typeparam name="T">追加するComponentの派生型</typeparam>
	/// <typeparam name="Args">Componentの生成に使用する引数の型</typeparam>
	/// <param name="args">Componentのコンストラクターへ渡す引数</param>
	/// <returns>追加したComponentへの参照</returns>
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

	/// <summary>
	/// 指定した型のComponentを検索する。
	/// </summary>
	/// <typeparam name="T">検索するComponentの派生型</typeparam>
	/// <returns>見つかったComponentへのポインター。存在しない場合はnullptr</returns>
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

	/// <summary>
	/// 指定した型のComponentを読み取り専用で検索する。
	/// </summary>
	/// <typeparam name="T">検索するComponentの派生型</typeparam>
	/// <returns>見つかったComponentへの読み取り専用ポインター。存在しない場合はnullptr</returns>
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

	/// <summary>
	/// 指定した型のComponentを切り離して削除する。
	/// 削除前にComponent::OnDetachを呼び出す。
	/// </summary>
	/// <typeparam name="T">削除するComponentの派生型</typeparam>
	/// <returns>削除した場合はtrue、対象が存在しない場合はfalse</returns>
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

	/// <summary>
	/// 有効なComponentの毎フレーム更新処理を登録順に実行する。
	/// </summary>
	void Update();

	/// <summary>
	/// 有効なComponentの固定間隔更新処理を登録順に実行する。
	/// </summary>
	void FixedUpdate();

	/// <summary>
	/// 有効なComponentの描画処理を登録順に実行する。
	/// </summary>
	void Draw();

	/// <summary>
	/// 所有しているすべてのComponentを切り離して削除する。
	/// </summary>
	void ClearComponents();

	/// <summary>
	/// デバッグ表示や検索に使用するGameObjectの名前を取得する。
	/// </summary>
	/// <returns>現在設定されている名前</returns>
	[[nodiscard]] const std::string &GetName() const {
		return name_;
	}

	/// <summary>
	/// GameObjectの名前を変更する。
	/// </summary>
	/// <param name="name">新しく設定する名前</param>
	void SetName(std::string name);

	/// <summary>
	/// GameObjectが更新と描画の対象になっているか取得する。
	/// </summary>
	/// <returns>有効な場合はtrue、無効な場合はfalse</returns>
	[[nodiscard]] bool IsActive() const {
		return active_;
	}

	/// <summary>
	/// GameObjectを更新と描画の対象にするか設定する。
	/// </summary>
	/// <param name="active">処理対象にする場合はtrue</param>
	void SetActive(bool active) {
		active_ = active;
	}

	/// <summary>
	/// 親子関係に対応した座標変換情報を取得する。
	/// </summary>
	/// <returns>編集可能なTransformNodeへの参照</returns>
	[[nodiscard]] TransformNode &GetTransform() {
		return transform_;
	}

	/// <summary>
	/// 親子関係に対応した座標変換情報を読み取り専用で取得する。
	/// </summary>
	/// <returns>読み取り専用のTransformNodeへの参照</returns>
	[[nodiscard]] const TransformNode &GetTransform() const {
		return transform_;
	}

	/// <summary>
	/// このGameObjectが所有しているComponentの数を取得する。
	/// </summary>
	/// <returns>登録済みComponentの数</returns>
	[[nodiscard]] size_t GetComponentCount() const {
		return components_.size();
	}

  private:
	/// <summary>
	/// 有効なComponentへ指定された処理を登録順に実行する。
	/// GameObjectが無効な場合は何も実行しない。
	/// </summary>
	/// <typeparam name="Callback">Componentを受け取る呼び出し可能オブジェクトの型</typeparam>
	/// <param name="callback">各Componentに対して実行する処理</param>
	template <typename Callback> void Dispatch(Callback &&callback) {
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

	/// <summary>
	/// Component処理中にComponent一覧が変更されないよう状態を検証する。
	/// 変更できない状態の場合は例外を送出する。
	/// </summary>
	void EnsureComponentsMutable() const;

	std::string name_;                                   // デバッグ表示や検索に使用するGameObjectの名前
	TransformNode transform_;                            // 親子関係を含むローカル座標とワールド座標
	std::vector<std::unique_ptr<Component>> components_; // このGameObjectが単独所有するComponent一覧
	bool active_ = true;                                 // Componentの更新と描画を実行するか
	bool dispatching_ = false;                           // Componentのコールバックを実行中か
};
