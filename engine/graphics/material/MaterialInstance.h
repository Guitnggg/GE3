#pragma once

#include "engine/math/RenderingTypes.h"
#include "engine/graphics/resource/TextureManager.h"

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;

enum class BlendMode : uint8_t {
	Opaque,   // 不透明として描画する
	Alpha,    // アルファ値で背景と合成する
	Additive, // 背景色へ加算して合成する
	Count     // ブレンド方式の総数
};

/// <summary>
/// 3D描画用のマテリアル定数、テクスチャ、ブレンド方式を共有可能な形で所有する。
/// </summary>
class MaterialInstance final {
  public:
	void Initialize(DirectXCommon *dxCommon, TextureHandle textureHandle);

	/// <summary>
	/// マテリアル定数とテクスチャを描画コマンドへ設定する。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList *commandList, const TextureManager &textureManager) const;

	/// <summary>
	/// 編集可能なマテリアル定数を返す。
	/// </summary>
	[[nodiscard]] Material &GetData();

	/// <summary>
	/// 読み取り専用のマテリアル定数を返す。
	/// </summary>
	[[nodiscard]] const Material &GetData() const;

	/// <summary>
	/// 描画に使用するテクスチャ番号を設定する。
	/// </summary>
	void SetTextureHandle(TextureHandle textureHandle) {
		textureHandle_ = textureHandle;
	}

	/// <summary>
	/// 描画に使用するテクスチャ番号を返す。
	/// </summary>
	[[nodiscard]] TextureHandle GetTextureHandle() const {
		return textureHandle_;
	}

	/// <summary>
	/// 描画時のブレンド方式を設定する。
	/// </summary>
	void SetBlendMode(BlendMode blendMode);

	/// <summary>
	/// 現在のブレンド方式を返す。
	/// </summary>
	[[nodiscard]] BlendMode GetBlendMode() const {
		return blendMode_;
	}

	/// <summary>
	/// GPU用マテリアル領域が作成済みかを返す。
	/// </summary>
	[[nodiscard]] bool IsInitialized() const {
		return dxCommon_ != nullptr;
	}

  private:
	DirectXCommon *dxCommon_ = nullptr;                // フレーム専用定数領域を確保する非所有参照
	Material data_{};                                  // GPUと競合しないCPU側の最新マテリアル
	TextureHandle textureHandle_{};                   // 描画に使用する世代付きテクスチャ参照
	BlendMode blendMode_ = BlendMode::Opaque;         // 描画時に使用するブレンド方式
};
