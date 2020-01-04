#pragma once

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <random>
#include <functional>

namespace gps {
	class PerlinNoise {
	public:
			PerlinNoise();

			// Get noise value in the XZ openGl plane
			float getNoise(glm::vec2 point);

			inline float interpolate(float a, float b, float t);

			inline float smoothCosine(float t);

			inline unsigned getPermutationSample(int x, int y);

	private:
		// Power of 2 for efficient modulo operations
		static const int randomSeedsCount = 512;

		static const int mod = randomSeedsCount - 1;

		// Look up table for corner vectors
		glm::vec2 randomSeedsLut[randomSeedsCount];

		// Permutations table for seeking appropiate corner vectors. Must be double to allow for
		// lookup on both dimensions
		int permutations[randomSeedsCount * 2];

		bool wasSetup;
	};
}