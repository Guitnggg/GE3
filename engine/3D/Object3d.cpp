#include "Object3d.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "Object3dCommon.h"
#include "TextureManager.h"
#include "engine/core/HResult.h"

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

void Object3d::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager, const std::string& directoryPath, const std::string& filename) {
	if (object3dCommon == nullptr || object3dCommon->GetDxCommon() == nullptr || textureManager == nullptr) {
		throw std::invalid_argument("Object3d requires Object3dCommon and TextureManager.");
	}
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;

	// OBJモデルを読み込む
	const ModelData modelData = LoadObjectFile(directoryPath, filename);
	if (modelData.material.textureFilePath.empty()) {
		throw std::runtime_error("The model material does not specify a diffuse texture: " + filename);
	}
	textureIndex_ = textureManager_->Load(modelData.material.textureFilePath);
	InitializeResources(modelData.vertices);
}

void Object3d::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager,
	const std::vector<VertexData>& vertices, uint32_t textureIndex) {
	if (object3dCommon == nullptr || object3dCommon->GetDxCommon() == nullptr || textureManager == nullptr) {
		throw std::invalid_argument("Object3d requires Object3dCommon and TextureManager.");
	}
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;
	textureIndex_ = textureIndex;
	InitializeResources(vertices);
}

void Object3d::InitializeResources(const std::vector<VertexData>& vertices) {
	auto* dxCommon = object3dCommon_->GetDxCommon();
	mesh_.Initialize(dxCommon, vertices);

	// マテリアル用定数バッファを作成する
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	HResult::ThrowIfFailed(
		materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_)),
		"Mapping the 3D object material buffer");
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = true;
	materialData_->uvTransform = MakeIdentity4x4();

	// 座標変換行列用定数バッファを作成する
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	HResult::ThrowIfFailed(
		transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_)),
		"Mapping the 3D object transformation buffer");
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// 平行光源用定数バッファを作成する
	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	HResult::ThrowIfFailed(
		directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_)),
		"Mapping the 3D object directional-light buffer");
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
}

void Object3d::Draw() const {
	if (object3dCommon_ == nullptr || textureManager_ == nullptr) {
		throw std::logic_error("Object3d is not initialized.");
	}
	auto* commandList = object3dCommon_->GetDxCommon()->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, GetTextureSrvHandleGPU());
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	mesh_.Draw(commandList);
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

TransformationMatrix* Object3d::GetTransformationMatrixData() const { return transformationMatrixData_; }

DirectionalLight* Object3d::GetDirectionalLightData() const { return directionalLightData_; }

Material* Object3d::GetMaterialData() const { return materialData_; }

void Object3d::SetTextureIndex(uint32_t textureIndex) { textureIndex_ = textureIndex; }

D3D12_GPU_DESCRIPTOR_HANDLE Object3d::GetTextureSrvHandleGPU() const {
	if (textureManager_ == nullptr) {
		throw std::logic_error("Object3d is not initialized.");
	}
	return textureManager_->GetSrvHandleGPU(textureIndex_);
}
