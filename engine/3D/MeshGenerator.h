#pragma once

#include <cstdint>
#include <vector>

#include "engine/core/Mymath.h"

namespace MeshGenerator {
	/// <summary>
	/// 指定分割数の単位球を、三角形リスト形式で生成する。
	/// </summary>
	std::vector<VertexData> CreateSphere(uint32_t subdivisions);
}
