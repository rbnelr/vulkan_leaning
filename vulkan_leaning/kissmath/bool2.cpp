// file was generated by kissmath.py at <TODO: add github link>
#include "bool2.hpp"

#include "uint8v2.hpp"
#include "bool4.hpp"
#include "int64v2.hpp"
#include "bool3.hpp"
#include "bool2.hpp"
#include "float2.hpp"
#include "int2.hpp"

namespace kissmath {
	//// forward declarations
	// typedef these because the _t suffix is kinda unwieldy when using these types often
	
	typedef uint8_t uint8;
	typedef int64_t int64;
	
	// Component indexing operator
	bool& bool2::operator[] (int i) {
		return arr[i];
	}
	
	// Component indexing operator
	bool const& bool2::operator[] (int i) const {
		return arr[i];
	}
	
	
	// uninitialized constructor
	bool2::bool2 () {
		
	}
	
	// sets all components to one value
	// implicit constructor -> float3(x,y,z) * 5 will be turned into float3(x,y,z) * float3(5) by to compiler to be able to execute operator*(float3, float3), which is desirable
	// and short initialization like float3 a = 0; works
	bool2::bool2 (bool all): x{all}, y{all} {
		
	}
	
	// supply all components
	bool2::bool2 (bool x, bool y): x{x}, y{y} {
		
	}
	
	// truncate vector
	bool2::bool2 (bool3 v): x{v.x}, y{v.y} {
		
	}
	
	// truncate vector
	bool2::bool2 (bool4 v): x{v.x}, y{v.y} {
		
	}
	
	//// Truncating cast operators
	
	
	//// Type cast operators
	
	
	// type cast operator
	bool2::operator int64v2 () const {
		return int64v2((int64)x, (int64)y);
	}
	
	// type cast operator
	bool2::operator int2 () const {
		return int2((int)x, (int)y);
	}
	
	// type cast operator
	bool2::operator float2 () const {
		return float2((float)x, (float)y);
	}
	
	// type cast operator
	bool2::operator uint8v2 () const {
		return uint8v2((uint8)x, (uint8)y);
	}
	
	//// reducing ops
	
	
	// all components are true
	bool all (bool2 v) {
		return v.x && v.y;
	}
	
	// any component is true
	bool any (bool2 v) {
		return v.x || v.y;
	}
	
	//// boolean ops
	
	
	bool2 operator! (bool2 v) {
		return bool2(!v.x, !v.y);
	}
	
	bool2 operator&& (bool2 l, bool2 r) {
		return bool2(l.x && r.x, l.y && r.y);
	}
	
	bool2 operator|| (bool2 l, bool2 r) {
		return bool2(l.x || r.x, l.y || r.y);
	}
	
	//// comparison ops
	
	
	// componentwise comparison returns a bool vector
	bool2 operator== (bool2 l, bool2 r) {
		return bool2(l.x == r.x, l.y == r.y);
	}
	
	// componentwise comparison returns a bool vector
	bool2 operator!= (bool2 l, bool2 r) {
		return bool2(l.x != r.x, l.y != r.y);
	}
	
	// vectors are equal, equivalent to all(l == r)
	bool equal (bool2 l, bool2 r) {
		return all(l == r);
	}
	
	// componentwise ternary (c ? l : r)
	bool2 select (bool2 c, bool2 l, bool2 r) {
		return bool2(c.x ? l.x : r.x, c.y ? l.y : r.y);
	}
}

