#pragma once

#include "engine/math/RenderingTypes.h"

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class TextureManager;

enum class BlendMode : uint8_t { Opaque, Alpha, Additive, Count };

/// <summary>3D描画用のマテリアル定数、テクスチャ、ブレンド方式を共有可能な形で所有する。</summary>
class MaterialInstance final {
  public:
	void Initialize(DirectXCommon *dxCommon, uint32_t textureIndex);
	void Bind(ID3D12GraphicsCommandList *commandList, const TextureManager &textureManager) const;

	[[nodiscard]] Material &GetData();
	[[nodiscard]] const Material &GetData() const;
	void SetTextureIndex(uint32_t textureIndex) {
		textureIndex_ = textureIndex;
	}
	[[nodiscard]] uint32_t GetTextureIndex() const {
		return textureIndex_;
	}
	void SetBlendMode(BlendMode blendMode);
	[[nodiscard]] BlendMode GetBlendMode() const {
		return blendMode_;
	}
	[[nodiscard]] bool IsInitialized() const {
		return data_ != nullptr;
	}

  private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	Material *data_ = nullptr;
	uint32_t textureIndex_ = 0;
	BlendMode blendMode_ = BlendMode::Opaque;
};
