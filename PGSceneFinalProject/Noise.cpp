
#include "Noise.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

namespace gps {

	Noise::Noise()
	{
	}

	float Noise::getHeightAt(float x, float z)
	{
		int octaves = 4;
		float height = 0.f;

		for (int i = 0; i < octaves; i++) {
			float freq = pow(2.f, i);
			float ampl = pow(1.f, -i);

			height += getInterpolatedNoise(x * freq, z * freq) * ampl;
		}

		return height;	
	}

	float Noise::getSimpleNoise(int x, int z)
	{
		int n = x + z * 57;

		n = (n << 13) ^ n;
		return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
	}

	float Noise::getSmoothNoise(float x, float z)
	{
		float corners = getSimpleNoise(x - 1, z - 1) + getSimpleNoise(x - 1, z + 1) + getSimpleNoise(x + 1, z - 1) + getSimpleNoise(x + 1, z + 1);
		float sides = getSimpleNoise(x, z - 1) + getSimpleNoise(x, z + 1) + getSimpleNoise(x - 1, z) + getSimpleNoise(x + 1, z);
		float center = getSimpleNoise(x, z);

		return corners / 16.f + sides / 8.f + center / 4.f;
	}

	float Noise::getInterpolatedNoise(float x, float z)
	{
		int x0 = (int)x;
		int z0 = (int)z;

		float tx = x - x0;
		float tz = z - z0;

		float noise00 = getSmoothNoise(x0, z0);
		float noise10 = getSmoothNoise(x0 + 1, z0);
		float noise01 = getSmoothNoise(x0, z0 + 1);
		float noise11 = getSmoothNoise(x0 + 1, z0 + 1);

		float xlerp0 = interpolate(noise00, noise10, tx);
		float xlerp1 = interpolate(noise01, noise11, tx);

		return interpolate(xlerp0, xlerp1, tz);
	}

	float Noise::interpolate(float a, float b, float t)
	{
		double theta = t * glm::pi<float>();
		float factor = (float)(1 - cos(theta)) * .5f;

		return (1.f - factor) * a + factor * b;
	}
}
