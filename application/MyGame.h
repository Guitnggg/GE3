#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
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
	std::unique_ptr<Object3d> sphere_;
	std::unique_ptr<Camera> camera_;

	// 読み込み済み音声とテクスチャのハンドル
	Audio::SoundHandle fanfareSound_{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2_{};
	uint32_t uvCheckerTexture_ = 0;
	uint32_t monsterBallTexture_ = 0;

	// 手続き生成する球体のメッシュ設定
	static constexpr uint32_t kSphereSubdivisions = 16;

	// スプライトが所有する定数バッファへの参照
	Material* materialDataSprite_ = nullptr;
	TransformationMatrix* transformationMatrixDataSprite_ = nullptr;

	// スプライトとUVの座標変換（3DのTransformは各Object3dが所有する）
	Transform transformSprite_{{1.0f, 1.0f, 1.0f}, {}, {}};
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
