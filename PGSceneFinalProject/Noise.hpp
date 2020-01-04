#pragma once

namespace gps {
	class Noise {
	
	public:
		Noise();
		float getHeightAt(float x, float y);

	private:
		float getSimpleNoise(int x, int z);
		float getSmoothNoise(float x, float z);
		float getInterpolatedNoise(float x, float z);
		float interpolate(float a, float b, float t);

	};
}