#include "Model.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "TextureManager.h"

namespace {
// OBJ解析中だけ使用し、GPU転送後は破棄する一時データ
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

size_t ResolveObjIndex(int32_t index, size_t count, const char* elementName) {
	// OBJでは正数が1始まり、負数が末尾からの相対位置、0は無効として定義される
	if (index == 0) { throw std::runtime_error(std::string("OBJ ") + elementName + " index must not be zero."); }
	const auto resolved = index > 0 ? static_cast<int64_t>(index - 1) : static_cast<int64_t>(count) + index;
	if (resolved < 0 || resolved >= static_cast<int64_t>(count)) {
		throw std::runtime_error(std::string("OBJ ") + elementName + " index is out of range.");
	}
	return static_cast<size_t>(resolved);
}

Vector3 CalculateFaceNormal(const Vector4& a, const Vector4& b, const Vector4& c) {
	// 法線を持たない面に使用する単位法線を、3頂点の外積から求める
	const Vector3 ab{b.x - a.x, b.y - a.y, b.z - a.z};
	const Vector3 ac{c.x - a.x, c.y - a.y, c.z - a.z};
	const Vector3 cross{ab.y * ac.z - ab.z * ac.y, ab.z * ac.x - ab.x * ac.z, ab.x * ac.y - ab.y * ac.x};
	const float lengthSquared = cross.x * cross.x + cross.y * cross.y + cross.z * cross.z;
	return lengthSquared > 0.0f ? Normalize(cross) : Vector3{0.0f, 0.0f, 1.0f};
}

MaterialData LoadMaterial(const std::string& directoryPath, const std::string& filename) {
	// MTLファイルから拡散反射テクスチャ（map_Kd）のパスを読み取る
	MaterialData material;
	std::ifstream file(directoryPath + "/" + filename);
	if (!file.is_open()) { throw std::runtime_error("Failed to open material file: " + directoryPath + "/" + filename); }
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string identifier;
		stream >> identifier;
		if (identifier == "map_Kd") {
			std::string textureFilename;
			stream >> textureFilename;
			material.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return material;
}

ModelData LoadObj(const std::string& directoryPath, const std::string& filename) {
	// OBJファイルを開き、位置・UV・法線・面・マテリアル参照を順番に解析する
	std::ifstream file(directoryPath + "/" + filename);
	if (!file.is_open()) { throw std::runtime_error("Failed to open OBJ file: " + directoryPath + "/" + filename); }

	ModelData model;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string identifier;
		stream >> identifier;
		if (identifier == "v") {
			// 頂点位置を右手系からエンジンの左手系へ変換する
			Vector4 position{};
			stream >> position.x >> position.y >> position.z;
			position.s = 1.0f;
			position.x *= -1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			// OBJとDirectXのUV原点の違いに合わせてV座標を反転する
			Vector2 texcoord{};
			stream >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			// 頂点位置と同様に法線も左手系へ変換する
			Vector3 normal{};
			stream >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			// v/vt/vn、v//vn、vのみの各形式と負インデックスを読み取る
			std::vector<VertexData> faceVertices;
			std::vector<bool> hasNormals;
			std::string definitionText;
			while (stream >> definitionText) {
				std::istringstream definition(definitionText);
				std::string positionText, texcoordText, normalText;
				std::getline(definition, positionText, '/');
				std::getline(definition, texcoordText, '/');
				std::getline(definition, normalText, '/');
				if (positionText.empty()) { throw std::runtime_error("OBJ face is missing a position index."); }
				VertexData vertex{};
				vertex.position = positions[ResolveObjIndex(std::stoi(positionText), positions.size(), "position")];
				if (!texcoordText.empty()) {
					vertex.texcoord = texcoords[ResolveObjIndex(std::stoi(texcoordText), texcoords.size(), "texcoord")];
				}
				const bool hasNormal = !normalText.empty();
				if (hasNormal) { vertex.normal = normals[ResolveObjIndex(std::stoi(normalText), normals.size(), "normal")]; }
				faceVertices.push_back(vertex);
				hasNormals.push_back(hasNormal);
			}
			if (faceVertices.size() < 3) { throw std::runtime_error("OBJ face has fewer than three vertices."); }
			// 法線が省略された頂点には面法線を補完する
			const Vector3 faceNormal = CalculateFaceNormal(faceVertices[2].position, faceVertices[1].position, faceVertices[0].position);
			for (size_t i = 0; i < faceVertices.size(); ++i) {
				if (!hasNormals[i]) { faceVertices[i].normal = faceNormal; }
			}
			// 三角形ファンで多角形を分割し、左手系に合わせて頂点順を反転する
			for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
				model.vertices.push_back(faceVertices[i + 1]);
				model.vertices.push_back(faceVertices[i]);
				model.vertices.push_back(faceVertices[0]);
			}
		} else if (identifier == "mtllib") {
			// OBJが参照するMTLファイルから既定テクスチャを取得する
			std::string materialFilename;
			stream >> materialFilename;
			model.material = LoadMaterial(directoryPath, materialFilename);
		}
	}
	return model;
}
}

void Model::InitializeFromObj(DirectXCommon* dxCommon, TextureManager* textureManager,
	const std::string& directoryPath, const std::string& filename) {
	// OBJとMTLを解析し、テクスチャをTextureManager経由で重複なく読み込む
	if (textureManager == nullptr) { throw std::invalid_argument("Model requires TextureManager."); }
	const ModelData modelData = LoadObj(directoryPath, filename);
	if (modelData.material.textureFilePath.empty()) {
		throw std::runtime_error("The model material does not specify a diffuse texture: " + filename);
	}
	// 解析結果をGPUへ転送する。modelDataは関数終了時に解放される
	defaultTextureIndex_ = textureManager->Load(modelData.material.textureFilePath);
	mesh_.Initialize(dxCommon, modelData.vertices);
}

void Model::InitializeFromVertices(DirectXCommon* dxCommon,
	const std::vector<VertexData>& vertices, uint32_t textureIndex) {
	// 球体など、ファイルを介さず生成された頂点列をそのままGPUへ転送する
	defaultTextureIndex_ = textureIndex;
	mesh_.Initialize(dxCommon, vertices);
}

// 実際の頂点バッファ設定と描画命令はMeshへ委譲する
void Model::Draw(ID3D12GraphicsCommandList* commandList) const { mesh_.Draw(commandList); }
