#pragma once

#include "engine/math/MathTypes.h"

#include <cstddef>
#include <vector>

/// <summary>
/// ローカルTransformと親子関係を保持し、必要なときだけワールド行列を再計算する。
/// 親子の所有権は持たず、GameObjectが寿命を管理する。
/// </summary>
class TransformNode final {
  public:
	/// <summary>
	/// 単位スケール、回転なし、原点位置のTransformNodeを生成する。
	/// </summary>
	TransformNode();

	/// <summary>
	/// 親子関係から自身を切り離して破棄する。
	/// 子ノード自体の寿命は管理しない。
	/// </summary>
	~TransformNode();

	/// <summary>
	/// 親子関係の参照を安全に保つため、コピーを禁止する。
	/// </summary>
	TransformNode(const TransformNode &) = delete;

	/// <summary>
	/// 親子関係の参照を安全に保つため、コピー代入を禁止する。
	/// </summary>
	TransformNode &operator=(const TransformNode &) = delete;

	/// <summary>
	/// 親ノードと子ノードが保持するアドレスを変えないため、ムーブを禁止する。
	/// </summary>
	TransformNode(TransformNode &&) = delete;

	/// <summary>
	/// 親ノードと子ノードが保持するアドレスを変えないため、ムーブ代入を禁止する。
	/// </summary>
	TransformNode &operator=(TransformNode &&) = delete;

	/// <summary>
	/// このノードの親を設定し、新しい親の子一覧へ登録する。
	/// nullptrを指定すると現在の親から切り離す。
	/// </summary>
	/// <param name="parent">新しい親ノードへの非所有ポインター</param>
	void SetParent(TransformNode *parent);

	/// <summary>
	/// 現在設定されている親ノードを取得する。
	/// </summary>
	/// <returns>親ノードへの非所有ポインター。親がない場合はnullptr</returns>
	[[nodiscard]] TransformNode *GetParent() {
		return parent_;
	}

	/// <summary>
	/// 現在設定されている親ノードを読み取り専用で取得する。
	/// </summary>
	/// <returns>親ノードへの読み取り専用ポインター。親がない場合はnullptr</returns>
	[[nodiscard]] const TransformNode *GetParent() const {
		return parent_;
	}

	/// <summary>
	/// このノードを親として参照している子ノードの数を取得する。
	/// </summary>
	/// <returns>登録されている子ノードの数</returns>
	[[nodiscard]] size_t GetChildCount() const {
		return children_.size();
	}

	/// <summary>
	/// 指定した位置に登録されている子ノードを取得する。
	/// </summary>
	/// <param name="index">取得する子ノードの番号</param>
	/// <returns>子ノードへの非所有ポインター</returns>
	[[nodiscard]] TransformNode *GetChild(size_t index);

	/// <summary>
	/// 指定した位置に登録されている子ノードを読み取り専用で取得する。
	/// </summary>
	/// <param name="index">取得する子ノードの番号</param>
	/// <returns>子ノードへの読み取り専用ポインター</returns>
	[[nodiscard]] const TransformNode *GetChild(size_t index) const;

	/// <summary>
	/// 親ノードを基準とした拡縮率を設定する。
	/// </summary>
	/// <param name="scale">各軸のローカル拡縮率</param>
	void SetLocalScale(const Vector3 &scale);

	/// <summary>
	/// 親ノードを基準としたオイラー回転角を設定する。
	/// </summary>
	/// <param name="rotation">各軸のローカル回転角</param>
	void SetLocalRotation(const Vector3 &rotation);

	/// <summary>
	/// 親ノードを基準とした位置を設定する。
	/// </summary>
	/// <param name="position">ローカル座標上の位置</param>
	void SetLocalPosition(const Vector3 &position);

	/// <summary>
	/// ローカルの拡縮、回転、位置をまとめて設定する。
	/// </summary>
	/// <param name="transform">新しく使用するローカルTransform</param>
	void SetLocalTransform(const Transform &transform);

	/// <summary>
	/// 親ノードを基準とした拡縮率を取得する。
	/// </summary>
	/// <returns>ローカル拡縮率への読み取り専用参照</returns>
	[[nodiscard]] const Vector3 &GetLocalScale() const {
		return local_.scale;
	}

	/// <summary>
	/// 親ノードを基準としたオイラー回転角を取得する。
	/// </summary>
	/// <returns>ローカル回転角への読み取り専用参照</returns>
	[[nodiscard]] const Vector3 &GetLocalRotation() const {
		return local_.rotate;
	}

	/// <summary>
	/// 親ノードを基準とした位置を取得する。
	/// </summary>
	/// <returns>ローカル位置への読み取り専用参照</returns>
	[[nodiscard]] const Vector3 &GetLocalPosition() const {
		return local_.translate;
	}

	/// <summary>
	/// ローカルの拡縮、回転、位置をまとめて取得する。
	/// </summary>
	/// <returns>ローカルTransformへの読み取り専用参照</returns>
	[[nodiscard]] const Transform &GetLocalTransform() const {
		return local_;
	}

	/// <summary>
	/// 親階層を反映したワールド行列を取得する。
	/// 変更があった場合だけ、取得時に行列を再計算する。
	/// </summary>
	/// <returns>キャッシュされたワールド行列への読み取り専用参照</returns>
	[[nodiscard]] const Matrix4x4 &GetWorldMatrix() const;

	/// <summary>
	/// 親階層を反映したワールド空間上の位置を取得する。
	/// </summary>
	/// <returns>ワールド行列から取り出した平行移動成分</returns>
	[[nodiscard]] Vector3 GetWorldPosition() const;

  private:
	/// <summary>
	/// 子一覧から指定したノードの参照を取り除く。
	/// 子ノード自体は破棄しない。
	/// </summary>
	/// <param name="child">取り除く子ノードへの非所有ポインター</param>
	void RemoveChild(TransformNode *child);

	/// <summary>
	/// 自身とすべての子孫へ、ワールド行列の再計算が必要であることを伝える。
	/// </summary>
	void MarkDirty();

	/// <summary>
	/// 指定したノードを親にした場合、親子関係が循環するか判定する。
	/// </summary>
	/// <param name="parent">親として設定しようとしているノード</param>
	/// <returns>循環が発生する場合はtrue</returns>
	[[nodiscard]] bool WouldCreateCycle(const TransformNode *parent) const;

	Transform local_{{1.0f, 1.0f, 1.0f}, {}, {}}; // 親を基準とした拡縮、回転、位置
	TransformNode *parent_ = nullptr;             // 現在の親ノードを指す非所有ポインター
	std::vector<TransformNode *> children_;       // このノードを親とする子ノードの非所有ポインター一覧
	mutable Matrix4x4 worldMatrix_{};             // 必要になったとき再計算するワールド行列のキャッシュ
	mutable bool dirty_ = true;                   // ワールド行列のキャッシュが古いかを示す状態
};
