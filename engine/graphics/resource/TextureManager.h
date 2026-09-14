#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <d3d12.h>
#include <wrl.h>

#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;
class SrvManager;

/// <summary>
/// テクスチャの読み込み、GPUへの転送、SRVハンドルの管理を行うクラス。
/// 同じパスのテクスチャは重複して読み込まない。
/// </summary>
class TextureManager {
public:
	/// <summary>
	/// DirectXとSRV管理クラスを受け取り、テクスチャ管理を初期化する。
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// <summary>
	/// 指定ファイルのテクスチャを読み込み、テクスチャ番号を返す。
	/// </summary>
	uint32_t Load(const std::string& filePath);

	/// <summary>
	/// テクスチャ番号に対応するGPU用SRVハンドルを取得する。
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex) const;

private:
	/// <summary>
	/// 読み込み済みテクスチャ1枚分の情報。
	/// </summary>
	struct TextureData {
		DirectX::TexMetadata metadata{};                 // 幅、高さ、フォーマットなどの画像情報
		Microsoft::WRL::ComPtr<ID3D12Resource> resource; // GPU上のテクスチャリソース
		uint32_t srvIndex = 0;                           // SrvManagerから割り当てられた番号
	};

	DirectXCommon* dxCommon_ = nullptr;                              // DirectX共通処理
	SrvManager* srvManager_ = nullptr;                               // SRVの割り当て管理
	std::vector<TextureData> textures_;                              // 読み込み済みテクスチャ一覧
	std::unordered_map<std::string, uint32_t> textureIndexMap_;      // ファイルパスからテクスチャ番号を検索する表
};
