#include "Object3d.h"

#include <cassert>
#include <fstream>
#include <cstring>
#include <sstream>

#include "Object3dCommon.h"

// 3Dオブジェクトのモデル、マテリアル、行列、ライト用リソースを初期化する
void Object3d::Initialize(Object3dCommon* object3dCommon) {
	assert(object3dCommon != nullptr);
	object3dCommon_ = object3dCommon;

	// OBJモデルを読み込む
	modelData_ = LoadObjectFile("resource", "axis.obj");

	// 頂点バッファを作成し、読み込んだ頂点データを転送する
	auto* dxCommon = object3dCommon_->GetDxCommon();
	vertexResource_ = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// マテリアル用定数バッファを作成する
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = true;
	materialData_->uvTransform = MakeIdentity4x4();

	// 座標変換行列用定数バッファを作成する
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// 平行光源用定数バッファを作成する
	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
}

// .mtlファイルからテクスチャファイルパスを読み込む
MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

// .objファイルを解析し、頂点データとマテリアルデータを作成する
ModelData Object3d::LoadObjectFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;

	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			// 頂点座標を読み込む
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.s = 1.0f;
			position.x *= -1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			// テクスチャ座標を読み込む
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			// 法線を読み込む
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			// 面情報を頂点データへ変換する
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position, texcoord, normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// 使用するマテリアルファイルを読み込む
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

D3D12_VERTEX_BUFFER_VIEW Object3d::GetVertexBufferView() const { return vertexBufferView_; }

ID3D12Resource* Object3d::GetMaterialResource() const { return materialResource_.Get(); }

ID3D12Resource* Object3d::GetTransformationMatrixResource() const { return transformationMatrixResource_.Get(); }

ID3D12Resource* Object3d::GetDirectionalLightResource() const { return directionalLightResource_.Get(); }

TransformationMatrix* Object3d::GetTransformationMatrixData() const { return transformationMatrixData_; }

DirectionalLight* Object3d::GetDirectionalLightData() const { return directionalLightData_; }

const ModelData& Object3d::GetModelData() const { return modelData_; }
