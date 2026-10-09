#pragma once

#include "engine/audio/Audio.h"

#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>

class Model;
class ModelManager;
class TextureManager;

/// <summary>
/// テクスチャ、モデル、音声をresource基準の統一パスで読み込む窓口。
/// 実データとキャッシュは各専用Managerが所有する。
/// </summary>
class AssetManager final {
  public:
	struct TextureHandle {
		static constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();
		uint32_t index = kInvalidIndex;

		[[nodiscard]] bool IsValid() const {
			return index != kInvalidIndex;
		}
	};

	AssetManager() = default;
	~AssetManager();
	AssetManager(const AssetManager &) = delete;
	AssetManager &operator=(const AssetManager &) = delete;

	/// <summary>
	/// 利用する専用Managerとアセットの基準ディレクトリを設定する。
	/// </summary>
	void Initialize(TextureManager *textureManager,
	                ModelManager *modelManager,
	                Audio *audio,
	                const std::filesystem::path &assetRoot = "resource");

	/// <summary>
	/// 参照を破棄し、モデルキャッシュを解放する。
	/// </summary>
	void Finalize();

	/// <summary>
	/// resource基準の画像を読み込み、型付きハンドルを返す。
	/// </summary>
	[[nodiscard]] TextureHandle LoadTexture(const std::filesystem::path &relativePath);

	/// <summary>
	/// resource基準のOBJモデルを読み込んで共有する。
	/// </summary>
	[[nodiscard]] std::shared_ptr<Model> LoadModel(const std::filesystem::path &relativePath);

	/// <summary>
	/// resource基準の音声を読み込み、再生用ハンドルを返す。
	/// </summary>
	[[nodiscard]] Audio::SoundHandle LoadSound(const std::filesystem::path &relativePath);

	/// <summary>
	/// 描画クラスへ渡すTextureManager内部の番号を取得する。
	/// </summary>
	[[nodiscard]] uint32_t GetTextureIndex(TextureHandle handle) const;

  private:
	[[nodiscard]] std::filesystem::path Resolve(const std::filesystem::path &path) const;
	void EnsureInitialized() const;

	TextureManager *textureManager_ = nullptr;
	ModelManager *modelManager_ = nullptr;
	Audio *audio_ = nullptr;
	std::filesystem::path assetRoot_;
	bool initialized_ = false;
};
