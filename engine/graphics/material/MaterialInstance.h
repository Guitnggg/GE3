#pragma once

#include "engine/math/RenderingTypes.h"

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class TextureManager;

enum class BlendMode : uint8_t {
	Opaque,   // 不透明として描画する
	Alpha,    // アルファ値で背景と合成する
	Additive, // 背景色へ加算して合成する
	Count     // ブレンド方式の総数
};

/// <summary>3D描画用のマテリアル定数、テクスチャ、ブレンド方式を共有可能な形で所有する。</summary>
class MaterialInstance final {
  public:
	void Initialize(DirectXCommon *dxCommon, uint32_t textureIndex);

	/// <summary>マテリアル定数とテクスチャを描画コマンドへ設定する。</summary>
	void Bind(ID3D12GraphicsCommandList *commandList, const TextureManager &textureManager) const;

	/// <summary>編集可能なマテリアル定数を返す。</summary>
	[[nodiscard]] Material &GetData();

	/// <summary>読み取り専用のマテリアル定数を返す。</summary>
	[[nodiscard]] const Material &GetData() const;

	/// <summary>描画に使用するテクスチャ番号を設定する。</summary>
	void SetTextureIndex(uint32_t textureIndex) {
		textureIndex_ = textureIndex;
	}

	/// <summary>描画に使用するテクスチャ番号を返す。</summary>
	[[nodiscard]] uint32_t GetTextureIndex() const {
		return textureIndex_;
	}

	/// <summary>描画時のブレンド方式を設定する。</summary>
	void SetBlendMode(BlendMode blendMode);

	/// <summary>現在のブレンド方式を返す。</summary>
	[[nodiscard]] BlendMode GetBlendMode() const {
		return blendMode_;
	}

	/// <summary>GPU用マテリアル領域が作成済みかを返す。</summary>
	[[nodiscard]] bool IsInitialized() const {
		return data_ != nullptr;
	}

  private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_; // マテリアル定数を格納するGPUバッファ
	Material *data_ = nullptr;                        // CPUから更新するマップ済み領域
	uint32_t textureIndex_ = 0;                       // 描画に使用するTextureManager内の番号
	BlendMode blendMode_ = BlendMode::Opaque;         // 描画時に使用するブレンド方式
};
