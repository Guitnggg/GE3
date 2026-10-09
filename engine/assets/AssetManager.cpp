#include "engine/assets/AssetManager.h"

#include "engine/3D/model/ModelManager.h"
#include "engine/graphics/resource/TextureManager.h"

#include <stdexcept>

AssetManager::~AssetManager() {
	Finalize();
}

void AssetManager::Initialize(TextureManager *textureManager,
                              ModelManager *modelManager,
                              Audio *audio,
                              const std::filesystem::path &assetRoot) {
	if (initialized_) {
		throw std::logic_error("AssetManager is already initialized.");
	}
	if (textureManager == nullptr || modelManager == nullptr || audio == nullptr) {
		throw std::invalid_argument("AssetManager requires TextureManager, ModelManager, and Audio.");
	}
	if (assetRoot.empty()) {
		throw std::invalid_argument("AssetManager requires a non-empty asset root.");
	}

	textureManager_ = textureManager;
	modelManager_ = modelManager;
	audio_ = audio;
	assetRoot_ = assetRoot.lexically_normal();
	initialized_ = true;
}

void AssetManager::Finalize() {
	if (!initialized_) {
		return;
	}

	// 共有モデルを先に解放し、モデルが参照するテクスチャはTextureManagerに残す
	modelManager_->Clear();
	textureManager_ = nullptr;
	modelManager_ = nullptr;
	audio_ = nullptr;
	assetRoot_.clear();
	initialized_ = false;
}

AssetManager::TextureHandle AssetManager::LoadTexture(const std::filesystem::path &relativePath) {
	EnsureInitialized();
	return TextureHandle{textureManager_->Load(Resolve(relativePath).generic_string())};
}

std::shared_ptr<Model> AssetManager::LoadModel(const std::filesystem::path &relativePath) {
	EnsureInitialized();
	const std::filesystem::path path = Resolve(relativePath);
	if (path.extension() != ".obj") {
		throw std::invalid_argument("AssetManager supports OBJ model files only.");
	}
	return modelManager_->Load(path.parent_path().generic_string(), path.filename().generic_string());
}

Audio::SoundHandle AssetManager::LoadSound(const std::filesystem::path &relativePath) {
	EnsureInitialized();
	return audio_->Load(Resolve(relativePath));
}

uint32_t AssetManager::GetTextureIndex(TextureHandle handle) const {
	EnsureInitialized();
	if (!handle.IsValid()) {
		throw std::invalid_argument("Invalid texture asset handle.");
	}
	return handle.index;
}

std::filesystem::path AssetManager::Resolve(const std::filesystem::path &path) const {
	if (path.empty()) {
		throw std::invalid_argument("Asset path must not be empty.");
	}
	return (path.is_absolute() ? path : assetRoot_ / path).lexically_normal();
}

void AssetManager::EnsureInitialized() const {
	if (!initialized_) {
		throw std::logic_error("AssetManager is not initialized.");
	}
}
