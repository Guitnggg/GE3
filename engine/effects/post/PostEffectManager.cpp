#include "engine/effects/post/PostEffectManager.h"

#include "engine/effects/post/IPostEffect.h"
#include "engine/graphics/render/RenderTargetManager.h"

#include <stdexcept>

void PostEffectManager::Initialize(DirectXCommon *directXCommon,
                                   RenderTargetManager *renderTargetManager,
                                   ShaderCompiler *shaderCompiler) {
	if (initialized_ || directXCommon == nullptr || renderTargetManager == nullptr || shaderCompiler == nullptr) {
		throw std::invalid_argument("PostEffectManager requires initialized rendering services.");
	}

	directXCommon_ = directXCommon;
	renderTargetManager_ = renderTargetManager;
	shaderCompiler_ = shaderCompiler;
	initialized_ = true;
}

void PostEffectManager::AddEffect(std::unique_ptr<IPostEffect> effect) {
	if (!initialized_) {
		throw std::logic_error("PostEffectManager is not initialized.");
	}
	if (!effect) {
		throw std::invalid_argument("Post effect must not be null.");
	}

	// Effect固有のGPUオブジェクトを構築してから所有権を受け取る。
	// 初期化が失敗した場合、チェーンは変更されない。
	effect->Initialize(directXCommon_, shaderCompiler_);
	effects_.push_back(std::move(effect));
}

void PostEffectManager::Apply() {
	if (!initialized_) {
		throw std::logic_error("PostEffectManager is not initialized.");
	}

	// 0番にはWorld描画結果が入っている。最初のEffectが読み取れる状態へ遷移する。
	uint32_t sourceIndex = RenderTargetManager::kWorldTextureIndex;
	renderTargetManager_->PrepareForSampling(sourceIndex);

	for (const auto &effect : effects_) {
		if (!effect->IsEnabled()) {
			continue;
		}

		// 入力と異なるTextureを出力先にすることで、同一Resourceの同時Read/Writeを避ける。
		const uint32_t destinationIndex =
		    (sourceIndex + 1u) % RenderTargetManager::kRenderTextureCount;
		renderTargetManager_->BeginPostEffectPass(destinationIndex);

		// 出力RTVはManagerが設定済み。Effectは入力SRVと固有PipelineだけをBindして描画する。
		effect->Draw(renderTargetManager_->GetTextureSrv(sourceIndex));

		// 今回の出力を、次のEffectが入力として読める状態へ切り替える。
		renderTargetManager_->PrepareForSampling(destinationIndex);
		sourceIndex = destinationIndex;
	}

	// Effectが0個なら0番、奇数個なら1番、偶数個なら0番が最終結果となる。
	renderTargetManager_->CompositeToSwapChain(sourceIndex);
}

void PostEffectManager::Finalize() noexcept {
	// EffectのPSO等を、依存先のDirectXCommonより先に解放する。
	effects_.clear();

	shaderCompiler_ = nullptr;
	renderTargetManager_ = nullptr;
	directXCommon_ = nullptr;
	initialized_ = false;
}
