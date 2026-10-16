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
	/// <summary>
	/// TextureManager内のテクスチャを型安全に参照する軽量ハンドル。
	/// </summary>
	struct TextureHandle {
		static constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max(); // 無効状態を表す予約番号
		uint32_t index = kInvalidIndex; // TextureManager内のテクスチャ番号

		/// <summary>
		/// 利用可能なテクスチャ番号を保持しているか判定する。
		/// </summary>
		[[nodiscard]] bool IsValid() const {
			return index != kInvalidIndex;
		}
	};

	/// <summary>
	/// アセット管理先を持たない未初期化状態で生成する。
	/// </summary>
	AssetManager() = default;

	/// <summary>
	/// 保持している参照と内部状態を破棄する。
	/// </summary>
	~AssetManager();

	/// <summary>
	/// 管理先との関連を一意に保つため、コピーを禁止する。
	/// </summary>
	AssetManager(const AssetManager &) = delete;

	/// <summary>
	/// 管理先との関連を一意に保つため、コピー代入を禁止する。
	/// </summary>
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
	/// <summary>
	/// 相対パスをアセット基準ディレクトリからの完全なパスへ解決する。
	/// </summary>
	[[nodiscard]] std::filesystem::path Resolve(const std::filesystem::path &path) const;

	/// <summary>
	/// 各専用Managerへの参照が設定済みであることを検証する。
	/// </summary>
	void EnsureInitialized() const;

	TextureManager *textureManager_ = nullptr; // テクスチャ読み込み先への非所有参照
	ModelManager *modelManager_ = nullptr;     // モデル読み込み先への非所有参照
	Audio *audio_ = nullptr;                   // 音声読み込み先への非所有参照
	std::filesystem::path assetRoot_;          // 相対パスの基準となるディレクトリ
	bool initialized_ = false;                 // 利用可能な初期化状態か
};
