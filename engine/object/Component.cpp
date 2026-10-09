#include "engine/object/Component.h"

#include "engine/object/GameObject.h"

#include <stdexcept>

GameObject &Component::GetGameObject() const {
	if (owner_ == nullptr) {
		throw std::logic_error("Component is not attached to a GameObject.");
	}
	return *owner_;
}
