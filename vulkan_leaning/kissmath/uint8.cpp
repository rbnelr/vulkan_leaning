// file was generated by kissmath.py at <TODO: add github link>
#include "uint8.hpp"

namespace kissmath {
	
	// wrap x into range [0,range)
	// negative x wrap back to +range unlike c++ % operator
	// negative range supported
	uint8 wrap (uint8 x, uint8 range) {
		uint8 modded = x % range;
		if (range > 0) {
			if (modded < 0) modded += range;
		} else {
			if (modded > 0) modded += range;
		}
		return modded;
	}
	
	// wrap x into [a,b) range
	uint8 wrap (uint8 x, uint8 a, uint8 b) {
		x -= a;
		uint8 range = b -a;
		
		uint8 modulo = wrap(x, range);
		
		return modulo + a;
	}
	
	// clamp x into range [a, b]
	// equivalent to min(max(x,a), b)
	uint8 clamp (uint8 x, uint8 a, uint8 b) {
		return min(max(x, a), b);
	}
	
	// clamp x into range [0, 1]
	// also known as saturate in hlsl
	uint8 clamp (uint8 x) {
		return min(max(x, uint8(0)), uint8(1));
	}
	
	// returns the greater value of a and b
	uint8 min (uint8 l, uint8 r) {
		return l <= r ? l : r;
	}
	
	// returns the smaller value of a and b
	uint8 max (uint8 l, uint8 r) {
		return l >= r ? l : r;
	}
	
	// equivalent to ternary c ? l : r
	// for conformity with vectors
	uint8 select (bool c, uint8 l, uint8 r) {
		return c ? l : r;
	}
	
}

