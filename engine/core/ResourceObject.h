/// <summary>
/// ID3D12ResourceのReleaseをデストラクタで行う簡易ラッパークラス
/// </summary>
class ResourceObject {
public:
	/// <summary>
	/// 管理対象のDirect3Dリソースを受け取る
	/// </summary>
	ResourceOnject(ID3D12Resource* resource)
		:resource_(resource){}
	
	/// <summary>
	/// 管理中のリソースを解放する
	/// </summary>
	~ResourceObject() {
		if (resource_) {
			resource_->Release();
		}
	}

	/// <summary>
	/// 管理中のリソースを取得する
	/// </summary>
	ID3D12Resource* Get() { return resource_; }

private:
	ID3D12Resoource* resource_  // 管理対象のDirect3Dリソース

};
