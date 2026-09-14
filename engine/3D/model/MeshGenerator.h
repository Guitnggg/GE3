#pragma once

#include <cstdint>
#include <vector>

#include "engine/math/Mymath.h"

namespace MeshGenerator {
	/// <summary>
	/// 原点を中心とする半径1の球体を、三角形リスト形式で生成する。
	/// 頂点には位置、UV座標、外向き法線が設定される。
	/// </summary>
	/// <param name="subdivisions">緯度方向と経度方向の分割数。3以上を指定する</param>
	/// <returns>GPU頂点バッファへ転送できる頂点列</returns>
	std::vector<VertexData> CreateSphere(uint32_t subdivisions);
}
