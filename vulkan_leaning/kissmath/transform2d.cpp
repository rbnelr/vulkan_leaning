// file was generated by kissmath.py at <TODO: add github link>
#include "transform2d.hpp"

#include <cmath>

namespace kissmath {
	
	
	float2x2 rotate2 (float ang) {
		float s = std::sin(ang), c = std::cos(ang);
		return float2x2(
						 c, -s,
						 s,  c
			   );
	}
	
	float2x2 scale (float2 v) {
		return float2x2(
						v.x,   0,
						  0, v.y
			   );
	}
	
	
}

