
#include "Helpers.hpp"

namespace gps {

	double coerce(double value, double lo, double hi) {
		if (value < lo)
			return lo;
		if (value > hi)
			return hi;
		return value;
	}

}