#include "MeshGenerator.h"

#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace MeshGenerator {
std::vector<VertexData> CreateSphere(uint32_t subdivisions) {
	// 球として成立しない分割数と、頂点数計算のオーバーフローを拒否する
	if (subdivisions < 3) {
		throw std::invalid_argument("Sphere subdivisions must be at least 3.");
	}

	constexpr uint64_t kVerticesPerCell = 6;
	const uint64_t vertexCount64 =
		static_cast<uint64_t>(subdivisions) * subdivisions * kVerticesPerCell;
	if (vertexCount64 > std::numeric_limits<size_t>::max() / sizeof(VertexData) ||
		vertexCount64 > std::numeric_limits<uint32_t>::max()) {
		throw std::overflow_error("Sphere mesh is too large.");
	}

	// 球面を緯度・経度方向へ等分する角度を計算する
	std::vector<VertexData> vertices(static_cast<size_t>(vertexCount64));
	constexpr float kPi = std::numbers::pi_v<float>;
	const float longitudeStep = kPi * 2.0f / static_cast<float>(subdivisions);
	const float latitudeStep = kPi / static_cast<float>(subdivisions);

	for (uint32_t latitudeIndex = 0; latitudeIndex < subdivisions; ++latitudeIndex) {
		const float latitude = -kPi / 2.0f + latitudeStep * static_cast<float>(latitudeIndex);
		for (uint32_t longitudeIndex = 0; longitudeIndex < subdivisions; ++longitudeIndex) {
			const uint32_t start =
				(latitudeIndex * subdivisions + longitudeIndex) * kVerticesPerCell;
			const float longitude = longitudeStep * static_cast<float>(longitudeIndex);

			// 球面上の角度から位置、UV、外向き法線を持つ頂点を作る
			auto createVertex = [subdivisions](float latitudeValue, float longitudeValue,
				uint32_t latitudeUvIndex, uint32_t longitudeUvIndex) {
				VertexData vertex{};
				vertex.position = {
					std::cos(latitudeValue) * std::cos(longitudeValue),
					std::sin(latitudeValue),
					std::cos(latitudeValue) * std::sin(longitudeValue),
					1.0f};
				vertex.texcoord = {
					static_cast<float>(longitudeUvIndex) / static_cast<float>(subdivisions),
					1.0f - static_cast<float>(latitudeUvIndex) / static_cast<float>(subdivisions)};
				vertex.normal = {vertex.position.x, vertex.position.y, vertex.position.z};
				return vertex;
			};

			const VertexData vertexA = createVertex(
				latitude, longitude, latitudeIndex, longitudeIndex);
			const VertexData vertexB = createVertex(
				latitude + latitudeStep, longitude, latitudeIndex + 1, longitudeIndex);
			const VertexData vertexC = createVertex(
				latitude, longitude + longitudeStep, latitudeIndex, longitudeIndex + 1);
			const VertexData vertexD = createVertex(
				latitude + latitudeStep, longitude + longitudeStep,
				latitudeIndex + 1, longitudeIndex + 1);

			// 緯度・経度で囲まれた四角形を2枚の三角形へ分割する
			vertices[start + 0] = vertexA;
			vertices[start + 1] = vertexB;
			vertices[start + 2] = vertexC;
			vertices[start + 3] = vertexC;
			vertices[start + 4] = vertexB;
			vertices[start + 5] = vertexD;
		}
	}

	return vertices;
}
}
