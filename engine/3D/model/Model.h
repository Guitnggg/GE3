#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Mesh.h"
#include "engine/graphics/resource/TextureManager.h"

class DirectXCommon;

/// <summary>
/// 共有可能なメッシュと既定テクスチャをまとめる3Dモデル資産。
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
	void InitializeFromObj(DirectXCommon *dxCommon,
	                       TextureManager *textureManager,
	                       const std::string &directoryPath,
	                       const std::string &filename);

	/// <summary>
	/// プログラム上で生成した頂点列と読み込み済みテクスチャからモデルを初期化する。
	/// </summary>
	/// <param name="dxCommon">GPUリソースの生成に使用するDirectX共通処理</param>
	/// <param name="vertices">三角形リスト形式の頂点データ</param>
	/// <param name="textureHandle">TextureManagerが発行した世代付きハンドル</param>
	void InitializeFromVertices(DirectXCommon *dxCommon,
	                            const std::vector<VertexData> &vertices,
	                            TextureHandle textureHandle);

	/// <summary>
	/// モデルが所有するメッシュを描画する。
	/// </summary>
	/// <param name="commandList">描画命令を記録するコマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList *commandList) const;

	/// <summary>
	/// モデルに設定された既定テクスチャ番号を取得する。
	/// </summary>
	/// <returns>TextureManager内のテクスチャ番号</returns>
	TextureHandle GetDefaultTextureHandle() const {
		return defaultTextureHandle_;
	}

  private:
	Mesh mesh_;                        // 複数のObject3dから共有されるGPU頂点データ
	TextureHandle defaultTextureHandle_{}; // MTLまたは生成時に指定された既定テクスチャ
};
