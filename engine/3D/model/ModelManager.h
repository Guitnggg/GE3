#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/math/Mymath.h"

class DirectXCommon;
class Model;
class TextureManager;

/// <summary>
/// モデル資産の生成とキャッシュを管理するクラス。
/// 同じパスのOBJは一度だけ読み込み、複数のObject3dへ共有する。
/// </summary>
class ModelManager {
public:
	/// <summary>モデル生成に必要なDirectXとテクスチャ管理クラスを設定する。</summary>
	/// <param name="dxCommon">GPUリソースの生成に使用するDirectX共通処理</param>
	/// <param name="textureManager">モデルのテクスチャを管理するクラス</param>
	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager);

	/// <summary>
	/// OBJモデルを読み込む。読み込み済みのパスならキャッシュ済みモデルを返す。
	/// </summary>
	/// <param name="directoryPath">OBJファイルが置かれたディレクトリ</param>
	/// <param name="filename">読み込むOBJファイル名</param>
	/// <returns>複数のObject3dで共有できるモデル</returns>
	std::shared_ptr<Model> Load(
		const std::string& directoryPath = "resource/models/axis", const std::string& filename = "axis.obj");

	/// <summary>
	/// 手続き生成した頂点列から、キャッシュしないモデルを生成する。
	/// </summary>
	/// <param name="vertices">三角形リスト形式の頂点データ</param>
	/// <param name="textureIndex">TextureManagerが発行したテクスチャ番号</param>
	/// <returns>生成したモデル</returns>
	std::shared_ptr<Model> Create(
		const std::vector<VertexData>& vertices, uint32_t textureIndex) const;

	/// <summary>
	/// 管理中のOBJモデルキャッシュを解放する。
	/// Object3dが参照中のモデルは、そのObject3dが破棄されるまで保持される。
	/// </summary>
	void Clear();

private:
	DirectXCommon* dxCommon_ = nullptr;       // モデルのGPUリソース生成に使用する
	TextureManager* textureManager_ = nullptr; // OBJモデルのテクスチャ読み込みに使用する
	std::unordered_map<std::string, std::shared_ptr<Model>> models_; // 正規化したパス別のモデルキャッシュ
};
