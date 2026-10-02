#pragma once

#include <Windows.h>
#include <dxcapi.h>
#include <string>
#include <wrl.h>

/// <summary>DXCの初期化とHLSLコンパイルを担当する。</summary>
class ShaderCompiler final {
public:
	void Initialize();
	Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::wstring& filePath, const wchar_t* profile) const;

private:
	Microsoft::WRL::ComPtr<IDxcUtils> utils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> compiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};
