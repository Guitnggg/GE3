#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

#include "engine/core/Mymath.h"

class SpriteCommon;
class TextureManager;

/// <summary>
/// 2Dスプライト固有の頂点、座標変換、UV変換、テクスチャ、描画処理を管理するクラス。
/// ゲーム側はTransformなどの表示設定を変更し、UpdateとDrawを呼ぶだけで描画できる。
/// </summary>
class Sprite {
public:
	/// <summary>
	/// スプライトに必要なGPUリソースと初期表示設定を生成する。
	/// </summary>
	/// <param name="spriteCommon">スプライト描画パイプラインを管理する共通処理</param>
	/// <param name="textureManager">描画テクスチャのSRVを管理するクラス</param>
	/// <param name="textureIndex">TextureManagerが発行した初期テクスチャ番号</param>
	void Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, uint32_t textureIndex);

	/// <summary>
	/// 現在の表示設定から頂点、画面座標行列、UV行列を更新する。
	/// </summary>
	/// <param name="viewportWidth">描画先の横幅</param>
	/// <param name="viewportHeight">描画先の縦幅</param>
	void Update(float viewportWidth, float viewportHeight);

	/// <summary>
	/// パイプラインと各GPUリソースを設定し、スプライトを描画する。
	/// </summary>
	void Draw() const;

	/// <summary>
	/// スプライトの位置・回転・拡縮を取得する。
	/// </summary>
	Transform& GetTransform() { return transform_; }
	const Transform& GetTransform() const { return transform_; }

	/// <summary>
	/// テクスチャに適用するUV座標変換を取得する。
	/// </summary>
	Transform& GetUvTransform() { return uvTransform_; }
	const Transform& GetUvTransform() const { return uvTransform_; }

	/// <summary>
	/// スプライトの表示サイズを設定する。
	/// </summary>
	/// <param name="size">ピクセル単位の横幅と縦幅</param>
	void SetSize(const Vector2& size);

	/// <summary>
	/// スプライトの基準点を設定する。左上が(0,0)、中央が(0.5,0.5)。
	/// </summary>
	/// <param name="anchorPoint">サイズに対する基準点の割合</param>
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	/// <summary>
	/// 描画に使用するテクスチャを変更する。
	/// </summary>
	/// <param name="textureIndex">TextureManagerが発行したテクスチャ番号</param>
	void SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }

private:
	/// <summary>
	/// サイズと基準点から矩形の4頂点を書き換える。
	/// </summary>
	void UpdateVertexData();

private:
	SpriteCommon* spriteCommon_ = nullptr;     // スプライト描画パイプラインの参照
	TextureManager* textureManager_ = nullptr; // テクスチャSRV管理の参照
	uint32_t textureIndex_ = 0;                // 現在描画するテクスチャ番号

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;               // 矩形の頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;                // 矩形のインデックスバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;             // 色とUV変換の定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_; // 画面座標行列の定数バッファ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{}; // 頂点バッファの描画情報
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};   // インデックスバッファの描画情報

	VertexData* vertexData_ = nullptr;							// マップ済み頂点データ書き込み先
	Material* materialData_ = nullptr;							// マップ済みマテリアル書き込み先
	TransformationMatrix* transformationMatrixData_ = nullptr;	// マップ済み行列書き込み先
	Transform transform_{{1.0f, 1.0f, 1.0f}, {}, {}};			// 画面上の拡縮・回転・位置
	Transform uvTransform_{{1.0f, 1.0f, 1.0f}, {}, {}};			// テクスチャのUV変換
	Vector2 size_{640.0f, 360.0f};								// ピクセル単位の表示サイズ
	Vector2 anchorPoint_{0.0f, 0.0f};							// サイズに対する描画基準点
};
