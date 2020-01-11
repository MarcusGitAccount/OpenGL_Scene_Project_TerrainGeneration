
#include "PerlinNoise.hpp"

namespace gps {

	PerlinNoise::PerlinNoise(void) {
		
		float epsilon = DBL_EPSILON;
		std::default_random_engine generator;
		std::uniform_real_distribution<float> floatDistribution; // eps->1 so we avoid 0 vectors
		auto rng = std::bind(floatDistribution, generator);

		for (int i = 0; i < randomSeedsCount; i++) {
			// Generate uniform(they lay on r=1 2d ball) corner vectors

			glm::vec2 corner;
			float length = 0;

			do {
				corner = glm::vec2(rng() * 2 - 1, rng() * 2 - 1);
				length = glm::length(corner);
			} while (length > 1);

			randomSeedsLut[i] = glm::normalize(corner);
			permutations[i] = i;
		}

		// Shuffle the permutations array
		std::uniform_int_distribution<unsigned> unsignedDistribution;
		auto randomIntGenerator = std::bind(unsignedDistribution, generator);

		for (int i = 0; i < randomSeedsCount - 1; i++) {
			unsigned swapIndex = randomIntGenerator() & mod;
			unsigned temp = permutations[swapIndex];

			permutations[swapIndex] = permutations[i];
			permutations[i] = temp;
		}

		// Copy the first half of the array in the second one to allow for 2D points sampling
		for (int i = 0; i < randomSeedsCount; i++) {
			permutations[i + randomSeedsCount] = permutations[i];
		}

		this->wasSetup = true;
	}

	float PerlinNoise::getNoise(glm::vec2 point) {
		if (!this->wasSetup) {
			return 0.0f;
		}

		int xl = (int)std::floor(point.x) & mod;
		int xh = (xl + 1) & mod;	

		int yl = (int)std::floor(point.y) & mod;
		int yh = (yl + 1) & mod;

		float tx = point.x - (int)std::floor(point.x);
		float ty = point.y - (int)std::floor(point.y);

		glm::vec2 dir00 = this->randomSeedsLut[getPermutationSample(xl, yl)];
		glm::vec2 dir01 = this->randomSeedsLut[getPermutationSample(xl, yh)];
		glm::vec2 dir10 = this->randomSeedsLut[getPermutationSample(xh, yl)];
		glm::vec2 dir11 = this->randomSeedsLut[getPermutationSample(xh, yh)];

		glm::vec2 corner00(point.x - xl, point.y - yl);
		glm::vec2 corner01(point.x - xl, point.y - yh);
		glm::vec2 corner10(point.x - xh, point.y - yl);
		glm::vec2 corner11(point.x - xh, point.y - yh);

		float xLerp1 = interpolate(glm::dot(corner00, dir00), glm::dot(corner00, dir00), tx);
		float xLerp2 = interpolate(glm::dot(corner01, dir01), glm::dot(corner01, dir01), tx);

		float yLerp = interpolate(xLerp1, xLerp2, ty);

		return yLerp;
	}

	inline float PerlinNoise::interpolate(float a, float b, float t) {
		float smoothie = smoothCosine(t);

		return (1.0f - smoothie) * a + smoothie * b;
	}

	inline float PerlinNoise::smoothCosine(float t) {
		return (1.0f - cos(t)) * .5;
		//return t * t * (3 - 2 * t);
	}

	inline unsigned PerlinNoise::getPermutationSample(int x, int y) {
		return permutations[permutations[x] + y];
	}
}