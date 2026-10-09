#pragma once

/// <summary>
/// スコープ終了時に生存中のDirect3Dオブジェクトを診断出力する。
/// </summary>
class D3DResourceLeakChecker {
  public:
	/// <summary>
	/// 生存しているDirect3D関連リソースをデバッグ出力へ報告する
	/// </summary>
	~D3DResourceLeakChecker();
};
