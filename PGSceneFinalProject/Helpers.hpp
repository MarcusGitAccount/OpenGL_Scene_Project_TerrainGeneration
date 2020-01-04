#pragma once

#include "glm/glm.hpp"

namespace gps {
	double coerce(double value, double lo, double hi);

	glm::vec3 getBarycentricCoords(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p);
}