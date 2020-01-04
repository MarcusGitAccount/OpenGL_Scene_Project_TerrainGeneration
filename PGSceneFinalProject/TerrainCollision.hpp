#pragma once

#include "Mesh.hpp"
#include "glm/glm.hpp"
#include <cmath>
#include "Helpers.hpp"

namespace gps {
	class TerrainCollision {
	public:
		TerrainCollision(void);

		TerrainCollision(Mesh terrainMesh);

		bool TerrainCollision::isAbove(glm::vec3 position);

		static const int xSize = 16;

		static const int zSize = 16;

		static const int xSubdivisions = 64;

		static const int zSubdivisions = 64;

		static const int xGridSize = xSubdivisions;

		static const int zGridSize = zSubdivisions;

	private:
		Mesh terrainMesh;

		glm::vec3 points[xGridSize + 1][zGridSize + 1];

		glm::vec3 normalizeCoords(glm::vec3 position);

		bool wasSetUp;

		float xHalf, zHalf;
	};
}