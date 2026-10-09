#pragma once

#include <Windows.h>
#include <dxcapi.h>
#include <string>
#include <wrl.h>

/// <summary>
/// DXCの初期化とHLSLコンパイルを担当する。
/// </summary>
class ShaderCompiler final {
  public:
	/// <summary>
	/// DXCユーティリティ、コンパイラ、インクルード処理を生成する。
	/// </summary>
	void Initialize();
	/// <summary>
	/// 指定したHLSLファイルをシェーダープロファイルに合わせてコンパイルする。
	/// </summary>
	/// <param name="filePath">コンパイルするHLSLファイルのパス</param>
	/// <param name="profile">vs_6_0やps_6_0などのシェーダープロファイル</param>
	/// <returns>コンパイル済みシェーダーバイトコード</returns>
	Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::wstring &filePath, const wchar_t *profile) const;

  private:
	Microsoft::WRL::ComPtr<IDxcUtils> utils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> compiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};
