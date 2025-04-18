#pragma once
#include <vector>
#include "VerTexData.h"
#include "MaterialData.h"

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};