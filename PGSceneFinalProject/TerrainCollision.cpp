
#include "TerrainCollision.hpp"
#include "glm/gtx/string_cast.hpp"

namespace gps {

	TerrainCollision::TerrainCollision(void) {
		this->wasSetUp = false;
	}

	TerrainCollision::TerrainCollision(Mesh terrainMesh) {
		this->terrainMesh = terrainMesh;
		this->xHalf = (float)xSize / 2.f;
		this->zHalf = (float)zSize / 2.f;

		for (auto& const vertex : terrainMesh.vertices) {
			glm::vec3 position = this->normalizeCoords(vertex.Position);

			int x = (int)(position.x);
			int z = (int)(position.z);

			points[z][x] = vertex.Position;
		}

		this->wasSetUp = true;
	}

	bool TerrainCollision::isAbove(glm::vec3 position) {
		if (!this->wasSetUp) {
			return 0.f;
		}

		glm::vec3 coords = this->normalizeCoords(position);
		float xPosNormalized = (position.x + xHalf) / (float)xSize;
		float zPosNormalized = (position.z + zHalf) / (float)zSize;

		int xCoord = (int)std::floor(xPosNormalized * xGridSize);
		int zCoord = (int)std::floor(zPosNormalized * zGridSize);

		if (xCoord < 0 || zCoord < 0) {
			return 0.f;
		}
		if (xCoord >= xGridSize || zCoord >= zGridSize) {
			return 0.f;
		}


		glm::vec3 a, b, c;

		a = points[zCoord][xCoord];
		if (xPosNormalized <= 1 - zPosNormalized) {
			// upper triangle
			b = points[zCoord][xCoord + 1];
			c = points[zCoord + 1][xCoord];
		} else {
			// bottom triangle
			b = points[zCoord + 1][xCoord];
			c = points[zCoord + 1][xCoord + 1];
		}

		//printf("(%d, %d)\n", xCoord, zCoord);
		glm::vec3 normal = glm::cross(c - a, b - a);
		float sign = glm::dot(normal, c - position);

		printf("Heights x, y: %5f %5f %5f. Camera height: %5f\n", a.y, b.y, c.y, position.y);
		std::cout << "Above? " << (sign > 0 ? "yes" : "no") << std::endl;

		return sign > 0; // vectors facing opposite directions
		//return true;
	}

	glm::vec3 TerrainCollision::normalizeCoords(glm::vec3 position) {
		float xPosNormalized = (position.x + xHalf) / (float)xSize;
		float zPosNormalized = (position.z + zHalf) / (float)zSize;

		int x = (int)(xPosNormalized * xGridSize);
		int z = (int)(zPosNormalized * zGridSize);

		return glm::vec3(x, position.y, z);
	}
}