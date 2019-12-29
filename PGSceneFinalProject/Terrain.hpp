#pragma once

#include "Mesh.hpp"
#include "PerlinNoise.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <functional>

namespace gps {
	class Terrain {
	
	public:
		Terrain();
		Terrain(int verticesCount);
		void Draw(gps::Shader shader);

	private:
		Mesh generateTerrain();
		
		Mesh terrainMesh;
		PerlinNoise perlinNoise;

		int verticesCount;
		bool wasGenerated;

		float getHeightAtCoord(int x, int y, int z);

		// Number of layers for noise wave generation
		static const int octaves = 5;
	};
}