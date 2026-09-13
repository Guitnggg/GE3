#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <wrl.h>
#include "application/Framework.h"
#include "engine/audio/Audio.h"
#include "engine/core/Mymath.h"

class Camera;
class Object3d;
class Sprite;

/// <summary>
/// このゲーム固有のデータと処理を管理するクラス。
/// </summary>
class MyGame : public Framework {
public:
	~MyGame() override;
	MyGame(const MyGame&) = delete;
	MyGame& operator=(const MyGame&) = delete;
	MyGame();

	/// <summary>
	///  初期化処理。DirectXやImGuiなどのゲーム共通機能を初期化した後に呼ばれる。
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 毎フレームの更新処理。入力や音声の状態を更新した後に呼ばれる。
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画処理。DirectXの描画開始とImGuiの描画を行った後に呼ばれる。
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 終了処理。DirectXやImGuiなどのゲーム共通機能を終了する前に呼ばれる。 
	/// </summary>
	void Finalize() override;

private:
	// このゲームで表示する2D・3Dオブジェクト
	std::unique_ptr<Sprite> sprite_;
	std::unique_ptr<Object3d> object3d_;
	std::unique_ptr<Camera> camera_;

	// 読み込み済み音声とテクスチャのハンドル
	Audio::SoundHandle fanfareSound_{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2_{};

	// 球体描画用の頂点バッファと定数バッファ
	static constexpr uint32_t kSphereSubdivisions = 16;
	uint32_t sphereVertexCount_ = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceSphere_;
	TransformationMatrix* wvpDataSphere_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere_;
	Material* materialDataSphere_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightSphereResource_;
	DirectionalLight* directionalLightSphereData_ = nullptr;

	// スプライトが所有する定数バッファへの参照
	Material* materialDataSprite_ = nullptr;
	TransformationMatrix* transformationMatrixDataSprite_ = nullptr;

	// 各オブジェクトの座標変換
	Transform transform_{{1.0f, 1.0f, 1.0f}, {}, {}};
	Transform transformSprite_{{1.0f, 1.0f, 1.0f}, {}, {}};
	Transform transformSphere_{{1.0f, 1.0f, 1.0f}, {}, {}};
	Transform uvTransformSprite_{{1.0f, 1.0f, 1.0f}, {}, {}};

	// ImGuiから変更する表示・動作設定
	bool textureChange_ = true;
	bool isRotate_ = false;
	bool isModel_ = false;
	bool isSphere_ = true;
	bool isSprite_ = false;

	// 二重解放を防ぐための初期化状態
	bool initialized_ = false;
};
