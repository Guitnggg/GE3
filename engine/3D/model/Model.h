#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Mesh.h"

class DirectXCommon;
class TextureManager;

/// <summary>
/// メッシュと既定テクスチャをまとめた、共有可能なモデル資産クラス。
/// 座標やライトなどの配置ごとの状態は持たず、Object3dがそれらを管理する。
/// </summary>
class Model {
public:
	/// <summary>
	/// OBJ・MTLファイルを読み込み、メッシュと既定テクスチャを初期化する。
	/// </summary>
	/// <param name="dxCommon">GPUリソースの生成に使用するDirectX共通処理</param>
	/// <param name="textureManager">MTLが参照するテクスチャの読み込み先</param>
	/// <param name="directoryPath">OBJ・MTLファイルが置かれたディレクトリ</param>
	/// <param name="filename">読み込むOBJファイル名</param>
	void InitializeFromObj(DirectXCommon* dxCommon, TextureManager* textureManager,
		const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// プログラム上で生成した頂点列と読み込み済みテクスチャからモデルを初期化する。
	/// </summary>
	/// <param name="dxCommon">GPUリソースの生成に使用するDirectX共通処理</param>
	/// <param name="vertices">三角形リスト形式の頂点データ</param>
	/// <param name="textureIndex">TextureManagerが発行したテクスチャ番号</param>
	void InitializeFromVertices(DirectXCommon* dxCommon,
		const std::vector<VertexData>& vertices, uint32_t textureIndex);

	/// <summary>
	/// モデルが所有するメッシュを描画する。
	/// </summary>
	/// <param name="commandList">描画命令を記録するコマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList* commandList) const;

	/// <summary>
	/// モデルに設定された既定テクスチャ番号を取得する。
	/// </summary>
	/// <returns>TextureManager内のテクスチャ番号</returns>
	uint32_t GetDefaultTextureIndex() const { return defaultTextureIndex_; }

private:
	Mesh mesh_;                       // 複数のObject3dから共有されるGPU頂点データ
	uint32_t defaultTextureIndex_ = 0; // MTLまたは生成時に指定された既定テクスチャ
};
