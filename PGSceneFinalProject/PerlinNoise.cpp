
#include "PerlinNoise.hpp"

namespace gps {

	PerlinNoise::PerlinNoise(void) {
		
		float epsilon = DBL_EPSILON;
		std::default_random_engine generator;
		std::uniform_real_distribution<float> floatDistribution(DBL_EPSILON, 1.0f); // eps->1 so we avoid 0 vectors
		auto rng = std::bind(floatDistribution, generator);

		for (int i = 0; i < randomSeedsCount; i++) {
			// Generate uniform(they lay on r=1 2d ball) corner vectors

			glm::vec3 corner(rng(), rng(), rng());

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

	float PerlinNoise::getNoise(glm::vec3 point) {
		if (!this->wasSetup) {
			return 0.0f;
		}

		int xl = (int)floor(point.x) & mod;
		int xh = (xl + 1) & mod;	

		int yl = (int)floor(point.y) & mod;
		int yh = (yl + 1) & mod;

		int zl = (int)floor(point.z) & mod;
		int zh = (zl + 1) & mod;

		float tx = point.x - (float)xl;
		float ty = point.y - (float)yl;
		float tz = point.z - (float)zl;

		glm::vec3 dir000 = this->randomSeedsLut[getPermutationSample(xl, yl, zl)];
		glm::vec3 dir001 = this->randomSeedsLut[getPermutationSample(xl, yl, zh)];
		glm::vec3 dir010 = this->randomSeedsLut[getPermutationSample(xl, yh, zl)];
		glm::vec3 dir011 = this->randomSeedsLut[getPermutationSample(xl, yh, zh)];

		glm::vec3 dir100 = this->randomSeedsLut[getPermutationSample(xh, yl, zl)];
		glm::vec3 dir101 = this->randomSeedsLut[getPermutationSample(xh, yl, zh)];
		glm::vec3 dir110 = this->randomSeedsLut[getPermutationSample(xh, yh, zl)];
		glm::vec3 dir111 = this->randomSeedsLut[getPermutationSample(xh, yh, zh)];

		glm::vec3 corner000(point.x - xl, point.y - yl, point.z - zl);
		glm::vec3 corner001(point.x - xl, point.y - yl, point.z - zh);
		glm::vec3 corner010(point.x - xl, point.y - yh, point.z - zl);
		glm::vec3 corner011(point.x - xl, point.y - yh, point.z - zh);

		glm::vec3 corner100(point.x - xh, point.y - yl, point.z - zl);
		glm::vec3 corner101(point.x - xh, point.y - yl, point.z - zh);
		glm::vec3 corner110(point.x - xh, point.y - yh, point.z - zl);
		glm::vec3 corner111(point.x - xh, point.y - yh, point.z - zh);

		// triliniar interpolation
		float xLerp1 = interpolate(glm::dot(corner000, dir000), glm::dot(corner100, dir100), tx);
		float xLerp2 = interpolate(glm::dot(corner001, dir001), glm::dot(corner101, dir101), tx);
		float xLerp3 = interpolate(glm::dot(corner010, dir010), glm::dot(corner110, dir110), tx);
		float xLerp4 = interpolate(glm::dot(corner011, dir011), glm::dot(corner111, dir111), tx);

		float yLerp1 = interpolate(xLerp1, xLerp2, ty);
		float yLerp2 = interpolate(xLerp3, xLerp4, ty);

		float zLerp = interpolate(yLerp1, yLerp2, tz);

		return zLerp;
	}

	inline float PerlinNoise::interpolate(float a, float b, float t) {
		float smoothie = smoothCosine(t);

		return (1.0f - smoothie) * a + smoothie * b;
	}

	inline float PerlinNoise::smoothCosine(float t) {
		return (1.0f - cos(t * glm::pi<float>())) / 2.0f;
	}

	inline unsigned PerlinNoise::getPermutationSample(int x, int y, int z) {
		return permutations[permutations[permutations[x] + y] + z];
	}
}