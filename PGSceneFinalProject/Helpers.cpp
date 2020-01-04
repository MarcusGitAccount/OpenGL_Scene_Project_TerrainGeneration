
#include "Helpers.hpp"
#include "glm/glm.hpp"
#include <cmath>

namespace gps {

	double coerce(double value, double lo, double hi) {
		if (value < lo)
			return lo;
		if (value > hi)
			return hi;
		return value;
	}

	glm::vec3 getBarycentricCoords(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p) {
		float t = (a.x - c.x) * (b.z - c.z) - (-b.x + c.x) * (a.z - c.z);

		float alpha = (b.z - c.z) * (p.x - c.x) + (c.x - b.x) * (p.z - c.z);
		float beta = (c.z - a.z) * (p.x - c.x) + (a.x - c.x) * (p.z - c.z);
		
		alpha /= t;
		beta /= t;

		float gamma = 1 - alpha - beta;

		return glm::vec3(alpha, beta, gamma);
	}

}