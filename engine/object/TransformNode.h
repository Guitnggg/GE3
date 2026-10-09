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
	TransformNode();
	~TransformNode();
	TransformNode(const TransformNode &) = delete;
	TransformNode &operator=(const TransformNode &) = delete;
	TransformNode(TransformNode &&) = delete;
	TransformNode &operator=(TransformNode &&) = delete;

	void SetParent(TransformNode *parent);
	[[nodiscard]] TransformNode *GetParent() {
		return parent_;
	}
	[[nodiscard]] const TransformNode *GetParent() const {
		return parent_;
	}
	[[nodiscard]] size_t GetChildCount() const {
		return children_.size();
	}
	[[nodiscard]] TransformNode *GetChild(size_t index);
	[[nodiscard]] const TransformNode *GetChild(size_t index) const;

	void SetLocalScale(const Vector3 &scale);
	void SetLocalRotation(const Vector3 &rotation);
	void SetLocalPosition(const Vector3 &position);
	void SetLocalTransform(const Transform &transform);

	[[nodiscard]] const Vector3 &GetLocalScale() const {
		return local_.scale;
	}
	[[nodiscard]] const Vector3 &GetLocalRotation() const {
		return local_.rotate;
	}
	[[nodiscard]] const Vector3 &GetLocalPosition() const {
		return local_.translate;
	}
	[[nodiscard]] const Transform &GetLocalTransform() const {
		return local_;
	}
	[[nodiscard]] const Matrix4x4 &GetWorldMatrix() const;
	[[nodiscard]] Vector3 GetWorldPosition() const;

  private:
	void RemoveChild(TransformNode *child);
	void MarkDirty();
	[[nodiscard]] bool WouldCreateCycle(const TransformNode *parent) const;

	Transform local_{{1.0f, 1.0f, 1.0f}, {}, {}}; // 親を基準にした拡縮・回転・位置
	TransformNode *parent_ = nullptr;             // 所有権を持たない親Transform
	std::vector<TransformNode *> children_;       // 所有権を持たない子Transform一覧
	mutable Matrix4x4 worldMatrix_{};             // 遅延計算して保持するワールド行列
	mutable bool dirty_ = true;                   // ワールド行列の再計算が必要か
};
