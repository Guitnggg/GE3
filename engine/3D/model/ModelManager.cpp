#include "ModelManager.h"

#include <filesystem>
#include <stdexcept>

#include "Model.h"

void ModelManager::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager) {
	// モデル生成中に使用する外部管理クラスが有効か検証して保持する
	if (dxCommon == nullptr || textureManager == nullptr) {
		throw std::invalid_argument("ModelManager requires DirectXCommon and TextureManager.");
	}
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
}

std::shared_ptr<Model> ModelManager::Load(const std::string& directoryPath, const std::string& filename) {
	// 初期化前の読み込みではGPUリソースを安全に生成できないため拒否する
	if (dxCommon_ == nullptr || textureManager_ == nullptr) {
		throw std::logic_error("ModelManager is not initialized.");
	}
	// "resource/./model.obj"などの表記揺れをまとめ、同じファイルの重複読み込みを防ぐ
	const std::string key = (std::filesystem::path(directoryPath) / filename).lexically_normal().generic_string();
	if (const auto found = models_.find(key); found != models_.end()) {
		return found->second;
	}

	// 未読み込みの場合だけファイル解析とGPUリソース生成を実行してキャッシュする
	auto model = std::make_shared<Model>();
	model->InitializeFromObj(dxCommon_, textureManager_, directoryPath, filename);
	models_.emplace(key, model);
	return model;
}

std::shared_ptr<Model> ModelManager::Create(
	const std::vector<VertexData>& vertices, uint32_t textureIndex) const {
	// 手続き生成モデルはファイルパスを持たないため、キャッシュへ登録せず呼び出し元へ返す
	if (dxCommon_ == nullptr) { throw std::logic_error("ModelManager is not initialized."); }
	auto model = std::make_shared<Model>();
	model->InitializeFromVertices(dxCommon_, vertices, textureIndex);
	return model;
}

// マネージャーが保持する参照だけを解放し、使用中モデルの寿命はshared_ptrへ任せる
void ModelManager::Clear() { models_.clear(); }
