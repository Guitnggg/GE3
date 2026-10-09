#include "engine/object/TransformNode.h"

#include "engine/math/MatrixMath.h"

#include <algorithm>
#include <stdexcept>

TransformNode::TransformNode() : worldMatrix_(MakeIdentity4x4()) {}

TransformNode::~TransformNode() {
	if (parent_ != nullptr) {
		parent_->RemoveChild(this);
	}
	for (TransformNode *child : children_) {
		child->parent_ = nullptr;
		child->MarkDirty();
	}
}

void TransformNode::SetParent(TransformNode *parent) {
	if (parent_ == parent) {
		return;
	}
	if (parent == this || WouldCreateCycle(parent)) {
		throw std::invalid_argument("Transform parent would create a hierarchy cycle.");
	}

	if (parent_ != nullptr) {
		parent_->RemoveChild(this);
	}
	parent_ = parent;
	if (parent_ != nullptr) {
		parent_->children_.push_back(this);
	}
	MarkDirty();
}

TransformNode *TransformNode::GetChild(size_t index) {
	if (index >= children_.size()) {
		throw std::out_of_range("Transform child index is out of range.");
	}
	return children_[index];
}

const TransformNode *TransformNode::GetChild(size_t index) const {
	if (index >= children_.size()) {
		throw std::out_of_range("Transform child index is out of range.");
	}
	return children_[index];
}

void TransformNode::SetLocalScale(const Vector3 &scale) {
	local_.scale = scale;
	MarkDirty();
}

void TransformNode::SetLocalRotation(const Vector3 &rotation) {
	local_.rotate = rotation;
	MarkDirty();
}

void TransformNode::SetLocalPosition(const Vector3 &position) {
	local_.translate = position;
	MarkDirty();
}

void TransformNode::SetLocalTransform(const Transform &transform) {
	local_ = transform;
	MarkDirty();
}

const Matrix4x4 &TransformNode::GetWorldMatrix() const {
	if (dirty_) {
		worldMatrix_ = MakeAffineMatrix(local_.scale, local_.rotate, local_.translate);
		if (parent_ != nullptr) {
			worldMatrix_ = Multiply(worldMatrix_, parent_->GetWorldMatrix());
		}
		dirty_ = false;
	}
	return worldMatrix_;
}

Vector3 TransformNode::GetWorldPosition() const {
	const Matrix4x4 &world = GetWorldMatrix();
	return {world.m[3][0], world.m[3][1], world.m[3][2]};
}

void TransformNode::RemoveChild(TransformNode *child) {
	children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
}

void TransformNode::MarkDirty() {
	if (dirty_) {
		return;
	}
	dirty_ = true;
	for (TransformNode *child : children_) {
		child->MarkDirty();
	}
}

bool TransformNode::WouldCreateCycle(const TransformNode *parent) const {
	for (const TransformNode *ancestor = parent; ancestor != nullptr; ancestor = ancestor->parent_) {
		if (ancestor == this) {
			return true;
		}
	}
	return false;
}
