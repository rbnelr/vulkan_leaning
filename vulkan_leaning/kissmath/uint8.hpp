// file was generated by kissmath.py at <TODO: add github link>
#pragma once

#include <cmath>
#include <cstdint>

namespace kissmath {
	typedef uint8_t uint8; // typedef this because the _t suffix is kinda unwieldy when using these types often
	
	// Use std math functions for these
	using std::abs;
	using std::floor;
	using std::ceil;
	using std::pow;
	using std::round;
	
	// wrap x into range [0,range)
	// negative x wrap back to +range unlike c++ % operator
	// negative range supported
	uint8 wrap (uint8 x, uint8 range);
	
	// wrap x into [a,b) range
	uint8 wrap (uint8 x, uint8 a, uint8 b);
	
	// clamp x into range [a, b]
	// equivalent to min(max(x,a), b)
	uint8 clamp (uint8 x, uint8 a, uint8 b);
	
	// clamp x into range [0, 1]
	// also known as saturate in hlsl
	uint8 clamp (uint8 x);
	
	// returns the greater value of a and b
	uint8 min (uint8 l, uint8 r);
	
	// returns the smaller value of a and b
	uint8 max (uint8 l, uint8 r);
	
	// equivalent to ternary c ? l : r
	// for conformity with vectors
	uint8 select (bool c, uint8 l, uint8 r);
	
	
}

