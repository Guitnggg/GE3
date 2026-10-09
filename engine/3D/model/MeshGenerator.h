#pragma once

#include <cstdint>
#include <vector>

#include "engine/math/RenderingTypes.h"

namespace MeshGenerator {
/// <summary>
/// 原点中心・半径1の球を、位置・UV・外向き法線を持つ三角形列として生成する。
/// </summary>
/// <param name="subdivisions">緯度方向と経度方向の分割数。3以上を指定する</param>
/// <returns>GPU頂点バッファへ転送できる頂点列</returns>
std::vector<VertexData> CreateSphere(uint32_t subdivisions);
} // namespace MeshGenerator
