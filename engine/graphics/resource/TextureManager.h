#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

#include "externals/DirectXTex/DirectXTex.h"
#include "engine/graphics/resource/SrvManager.h"

class DirectXCommon;

/// <summary>
/// TextureManager内のスロットと世代を組み合わせた型安全な参照。
/// </summary>
struct TextureHandle {
	static constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();
	uint32_t index = kInvalidIndex;
	uint32_t generation = 0;

	[[nodiscard]] bool IsValid() const noexcept {
		return index != kInvalidIndex && generation != 0;
	}
	bool operator==(const TextureHandle &) const noexcept = default;
};

/// <summary>
/// 画像ファイルをGPUへ転送し、パスごとのSRVをキャッシュする。
/// </summary>
class TextureManager {
  public:
	/// <summary>
	/// DirectXとSRV管理クラスを受け取り、テクスチャ管理を初期化する。
	/// </summary>
	void Initialize(DirectXCommon *dxCommon, SrvManager *srvManager);

	/// <summary>
	/// 指定ファイルのテクスチャを読み込み、世代付きハンドルを返す。
	/// </summary>
	[[nodiscard]] TextureHandle Load(const std::string &filePath);

	/// <summary>
	/// テクスチャを無効化し、GPU完了後にリソースとSRVを再利用可能にする。
	/// </summary>
	void Unload(TextureHandle handle);

	/// <summary>
	/// GPUが使用を終えた保留リソースを解放する。
	/// </summary>
	void CollectGarbage();

	/// <summary>
	/// ハンドルが現在読み込み済みのテクスチャを指しているか返す。
	/// </summary>
	[[nodiscard]] bool IsValid(TextureHandle handle) const noexcept;

	/// <summary>
	/// テクスチャハンドルに対応するGPU用SRVハンドルを取得する。
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(TextureHandle handle) const;

  private:
	/// <summary>
	/// CPU側のファイル読込とデコードが完了した中間データ。
	/// GPUオブジェクトを含まないため、将来はワーカースレッドで生成できる。
	/// </summary>
	struct DecodedTexture {
		DirectX::ScratchImage mipImages;
		std::string path;
	};

	/// <summary>
	/// 読み込み済みテクスチャ1枚分の情報。
	/// </summary>
	struct TextureData {
		DirectX::TexMetadata metadata{};                 // 幅、高さ、フォーマットなどの画像情報
		Microsoft::WRL::ComPtr<ID3D12Resource> resource; // GPU上のテクスチャリソース
		SrvHandle srvHandle{};                           // SrvManagerから割り当てられた世代付き参照
		std::string path;                                // キャッシュ表から削除する正規化済みパス
		uint32_t referenceCount = 1;                     // 同じパスをLoadした利用者の数
	};
	struct TextureSlot {
		uint32_t generation = 1;
		std::optional<TextureData> data;
	};
	struct PendingRelease {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource; // GPU完了まで寿命を延長する本体
		SrvHandle srvHandle{};                           // 同じ時点まで再利用を禁止するSRV
		uint64_t fenceValue = 0;                         // この値の完了後に安全に解放できる
	};

	[[nodiscard]] const TextureData &Require(TextureHandle handle) const;
	[[nodiscard]] DecodedTexture Decode(const std::string &filePath) const;
	[[nodiscard]] TextureData Upload(DecodedTexture decodedTexture);

	DirectXCommon *dxCommon_ = nullptr;                               // DirectX共通処理
	SrvManager *srvManager_ = nullptr;                                // SRVの割り当て管理
	std::vector<TextureSlot> textureSlots_;                           // 再利用可能なテクスチャ格納場所
	std::vector<uint32_t> freeTextureIndices_;                        // 空いているテクスチャ番号
	std::vector<PendingRelease> pendingReleases_;                     // GPU完了を待っているリソース
	std::unordered_map<std::string, TextureHandle> textureHandleMap_; // パスから現在のハンドルを引く表
};
