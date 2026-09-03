#pragma once

#include <d3d12.h>
#include <wrl.h>

/// <summary>
/// ID3D12ResourceのReleaseをデストラクタで行う簡易ラッパークラス
/// </summary>
class ResourceObject {
public:
	/// <summary>
	/// 管理対象のDirect3Dリソースを受け取る
	/// </summary>
	explicit ResourceObject(ID3D12Resource* resource)
		:resource_(resource){}
	ResourceObject(const ResourceObject&) = delete;
	ResourceObject& operator=(const ResourceObject&) = delete;
	ResourceObject(ResourceObject&&) noexcept = default;
	ResourceObject& operator=(ResourceObject&&) noexcept = default;

	/// <summary>
	/// 管理中のリソースを取得する
	/// </summary>
	ID3D12Resource* Get() const { return resource_.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;  // 管理対象のDirect3Dリソース

};
