#include "engine/graphics/shader/ShaderCompiler.h"

#include "engine/core/diagnostics/HResult.h"
#include "engine/core/diagnostics/Logger.h"
#include "engine/core/utility/StringUtility.h"

#include <format>
#include <stdexcept>

void ShaderCompiler::Initialize() {
	if (utils_ || compiler_ || includeHandler_) {
		throw std::logic_error("ShaderCompiler is already initialized.");
	}
	HResult::ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils_)), "Creating DXC utilities");
	HResult::ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler_)), "Creating the DXC compiler");
	HResult::ThrowIfFailed(utils_->CreateDefaultIncludeHandler(&includeHandler_), "Creating the DXC include handler");
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Compile(const std::wstring &filePath, const wchar_t *profile) const {
	if (!utils_ || !compiler_ || !includeHandler_) {
		throw std::logic_error("ShaderCompiler is not initialized.");
	}
	if (filePath.empty() || profile == nullptr) {
		throw std::invalid_argument("ShaderCompiler requires a path and profile.");
	}

	Logger::Log(
	    StringUtility::ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> source;
	HResult::ThrowIfFailed(utils_->LoadFile(filePath.c_str(), nullptr, &source), "Loading a shader file");
	DxcBuffer sourceBuffer{source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_UTF8};
	LPCWSTR arguments[] = {
	    filePath.c_str(),
	    L"-E",
	    L"main",
	    L"-T",
	    profile,
	    L"-Zi",
	    L"-Qembed_debug",
	    L"-Od",
	    L"-Zpr",
	};
	Microsoft::WRL::ComPtr<IDxcResult> result;
	HResult::ThrowIfFailed(
	    compiler_->Compile(&sourceBuffer, arguments, _countof(arguments), includeHandler_.Get(), IID_PPV_ARGS(&result)),
	    "Compiling a shader");

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
	HResult::ThrowIfFailed(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr),
	                       "Reading shader compiler diagnostics");
	if (errors && errors->GetStringLength() != 0) {
		Logger::Log(errors->GetStringPointer());
		throw std::runtime_error(std::string("Shader compilation failed: ") + errors->GetStringPointer());
	}

	Microsoft::WRL::ComPtr<IDxcBlob> shader;
	HResult::ThrowIfFailed(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr),
	                       "Getting compiled shader output");
	Logger::Log(
	    StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
	return shader;
}
