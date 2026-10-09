#include "application/environment/StageEnvironment.h"

#include "engine/3D/camera/Camera.h"
#include "engine/3D/model/Model.h"
#include "engine/3D/object/Object3d.h"

#include <stdexcept>

namespace {
constexpr uint32_t kSegmentCount = 7;
constexpr float kScale = 2.0f;
constexpr float kSegmentLength = 22.0f;
constexpr float kStartZ = -5.0f;
} // namespace

StageEnvironment::~StageEnvironment() {
	Finalize();
}

void StageEnvironment::Initialize(Object3dCommon *object3dCommon,
                                  TextureManager *textureManager,
                                  const std::shared_ptr<Model> &mapModel) {
	if (!object3dCommon || !textureManager || !mapModel || !segments_.empty()) {
		throw std::invalid_argument("StageEnvironment requires services, a model, and an empty state.");
	}
	mapModel_ = mapModel;
	for (uint32_t i = 0; i < kSegmentCount; ++i) {
		auto segment = std::make_unique<Object3d>();
		segment->Initialize(object3dCommon, textureManager, mapModel_);
		segment->GetTransform().scale = {kScale, kScale, kScale};
		segment->GetMaterialData()->color = {0.38f, 0.48f, 0.68f, 1.0f};
		segment->GetDirectionalLightData()->direction = {-0.35f, -1.0f, 0.25f};
		segment->GetDirectionalLightData()->intensity = 1.25f;
		segments_.push_back(std::move(segment));
	}
	Reset();
}

void StageEnvironment::Reset() {
	for (size_t i = 0; i < segments_.size(); ++i) {
		segments_[i]->GetTransform().translate = {0.0f, -4.0f, kStartZ + kSegmentLength * i};
	}
}

void StageEnvironment::Update(const Camera &camera, float cameraZ) {
	const float loopLength = kSegmentLength * static_cast<float>(segments_.size());
	for (auto &segment : segments_) {
		if (segment->GetTransform().translate.z < cameraZ - kSegmentLength) {
			segment->GetTransform().translate.z += loopLength;
		}
		segment->Update(camera);
	}
}

void StageEnvironment::Draw() const {
	for (const auto &segment : segments_) {
		segment->Draw();
	}
}

void StageEnvironment::Finalize() {
	segments_.clear();
	mapModel_.reset();
}
