
class ResourceObject {
public:
	ResourceOnject(ID3D12Resource* resource)
		:resource_(resource){}
	
	~ResourceObject() {
		if (resource_) {
			resource_->Release();
		}
	}
	ID3D12Resource* Get() { return resource_; }
private:
	ID3D12Resoource* resource_

};