#pragma once

/// <summary>
/// Direct3Dリソースのリークを終了時に確認するクラス
/// </summary>
class D3DResourceLeakChecker
{
public:
	/// <summary>
	/// 生存しているDirect3D関連リソースをデバッグ出力へ報告する
	/// </summary>
	~D3DResourceLeakChecker();
};
