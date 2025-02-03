#pragma once

#include "DirectXCommon.h"

class SpriteCommon
{
public:

    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // ルートシグネチャの作成
    void CreateRootSignature();

    // グラフィックスパイプラインの生成
    void CreateGraphicsPipeLine();

    // 共通描画設定
    void CreateCommonRendering();

public:

    // getter
    DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:

    // dxCommon
    DirectXCommon* dxCommon_;
};

