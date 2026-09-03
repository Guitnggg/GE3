#include "Object3d.h"

#include <cassert>
#include <fstream>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "Object3dCommon.h"

// 3Dオブジェクトのモデル、マテリアル、行列、ライト用リソースを初期化する
namespace {
size_t ResolveObjIndex(int32_t index, size_t count, const char* elementName) {
	if (index == 0) {
		throw std::runtime_error(std::string("OBJ ") + elementName + " index must not be zero.");
	}
	const auto resolved = index > 0
		? static_cast<int64_t>(index - 1)
		: static_cast<int64_t>(count) + index;
	if (resolved < 0 || resolved >= static_cast<int64_t>(count)) {
		throw std::runtime_error(std::string("OBJ ") + elementName + " index is out of range.");
	}
	return static_cast<size_t>(resolved);
}

Vector3 CalculateFaceNormal(const Vector4& a, const Vector4& b, const Vector4& c) {
	const Vector3 ab{b.x - a.x, b.y - a.y, b.z - a.z};
	const Vector3 ac{c.x - a.x, c.y - a.y, c.z - a.z};
	const Vector3 cross{
		ab.y * ac.z - ab.z * ac.y,
		ab.z * ac.x - ab.x * ac.z,
		ab.x * ac.y - ab.y * ac.x};
	const float lengthSquared = cross.x * cross.x + cross.y * cross.y + cross.z * cross.z;
	return lengthSquared > 0.0f ? Normalize(cross) : Vector3{0.0f, 0.0f, 1.0f};
}
}

void Object3d::Initialize(Object3dCommon* object3dCommon, const std::string& directoryPath, const std::string& filename) {
	if (object3dCommon == nullptr || object3dCommon->GetDxCommon() == nullptr) {
		throw std::invalid_argument("Object3d requires a valid Object3dCommon instance.");
	}
	object3dCommon_ = object3dCommon;

	// OBJモデルを読み込む
	modelData_ = LoadObjectFile(directoryPath, filename);

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
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open material file: " + directoryPath + "/" + filename);
	}

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
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open OBJ file: " + directoryPath + "/" + filename);
	}

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
			// 三角形・四角形以上、v/vt/vn・v//vn・v、負インデックスを扱う
			std::vector<VertexData> faceVertices;
			std::vector<bool> hasNormals;
			std::string vertexDefinition;
			while (s >> vertexDefinition) {
				std::istringstream definition(vertexDefinition);
				std::string positionIndexText;
				std::string texcoordIndexText;
				std::string normalIndexText;
				std::getline(definition, positionIndexText, '/');
				std::getline(definition, texcoordIndexText, '/');
				std::getline(definition, normalIndexText, '/');

				if (positionIndexText.empty()) {
					throw std::runtime_error("OBJ face is missing a position index.");
				}
				VertexData vertex{};
				vertex.position = positions[ResolveObjIndex(std::stoi(positionIndexText), positions.size(), "position")];
				if (!texcoordIndexText.empty()) {
					vertex.texcoord = texcoords[ResolveObjIndex(std::stoi(texcoordIndexText), texcoords.size(), "texcoord")];
				}
				const bool hasNormal = !normalIndexText.empty();
				if (hasNormal) {
					vertex.normal = normals[ResolveObjIndex(std::stoi(normalIndexText), normals.size(), "normal")];
				}
				faceVertices.push_back(vertex);
				hasNormals.push_back(hasNormal);
			}

			if (faceVertices.size() < 3) {
				throw std::runtime_error("OBJ face has fewer than three vertices.");
			}
			// 出力時の反転後の頂点順と同じ向きで法線を生成する
			const Vector3 faceNormal = CalculateFaceNormal(faceVertices[2].position, faceVertices[1].position, faceVertices[0].position);
			for (size_t i = 0; i < faceVertices.size(); ++i) {
				if (!hasNormals[i]) {
					faceVertices[i].normal = faceNormal;
				}
			}

			// 三角形ファンへ分割し、右手系から左手系への変換に合わせて頂点順を反転する
			for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
				modelData.vertices.push_back(faceVertices[i + 1]);
				modelData.vertices.push_back(faceVertices[i]);
				modelData.vertices.push_back(faceVertices[0]);
			}
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
