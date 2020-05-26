// file was generated by kissmath.py at <TODO: add github link>
#pragma once

#include "float.hpp"

namespace kissmath {
	//// forward declarations
	
	struct uint8v2;
	struct int64v2;
	struct float4;
	struct float3;
	struct bool2;
	struct int2;
	
	struct float2 {
		union { // Union with named members and array members to allow vector[] operator, not 100% sure that this is not undefined behavoir, but I think all compilers definitely don't screw up this use case
			struct {
				float	x, y;
			};
			float		arr[2];
		};
		
		// Component indexing operator
		float& operator[] (int i);
		
		// Component indexing operator
		float const& operator[] (int i) const;
		
		
		// uninitialized constructor
		inline float2 () = default;

		// sets all components to one value
		// implicit constructor -> float3(x,y,z) * 5 will be turned into float3(x,y,z) * float3(5) by to compiler to be able to execute operator*(float3, float3), which is desirable
		// and short initialization like float3 a = 0; works
		constexpr float2 (float all): x{all}, y{all} {

		}

		// supply all components
		constexpr float2 (float x, float y): x{x}, y{y} {

		}

		// truncate vector
		float2 (float3 v);

		// truncate vector
		float2 (float4 v);
		
		
		//// Truncating cast operators
		
		
		//// Type cast operators
		
		// type cast operator
		explicit operator int64v2 () const;
		
		// type cast operator
		explicit operator int2 () const;
		
		// type cast operator
		explicit operator uint8v2 () const;
		
		// type cast operator
		explicit operator bool2 () const;
		
		
		// componentwise arithmetic operator
		float2 operator+= (float2 r);
		
		// componentwise arithmetic operator
		float2 operator-= (float2 r);
		
		// componentwise arithmetic operator
		float2 operator*= (float2 r);
		
		// componentwise arithmetic operator
		float2 operator/= (float2 r);
		
	};
	
	//// arthmethic ops
	
	float2 operator+ (float2 v);
	
	float2 operator- (float2 v);
	
	float2 operator+ (float2 l, float2 r);
	
	float2 operator- (float2 l, float2 r);
	
	float2 operator* (float2 l, float2 r);
	
	float2 operator/ (float2 l, float2 r);
	
	
	//// bitwise ops
	
	
	//// comparison ops
	
	// componentwise comparison returns a bool vector
	bool2 operator< (float2 l, float2 r);
	
	// componentwise comparison returns a bool vector
	bool2 operator<= (float2 l, float2 r);
	
	// componentwise comparison returns a bool vector
	bool2 operator> (float2 l, float2 r);
	
	// componentwise comparison returns a bool vector
	bool2 operator>= (float2 l, float2 r);
	
	// componentwise comparison returns a bool vector
	bool2 operator== (float2 l, float2 r);
	
	// componentwise comparison returns a bool vector
	bool2 operator!= (float2 l, float2 r);
	
	// vectors are equal, equivalent to all(l == r)
	bool equal (float2 l, float2 r);
	
	// componentwise ternary (c ? l : r)
	float2 select (bool2 c, float2 l, float2 r);
	
	
	//// misc ops
	// componentwise absolute
	float2 abs (float2 v);
	
	// componentwise minimum
	float2 min (float2 l, float2 r);
	
	// componentwise maximum
	float2 max (float2 l, float2 r);
	
	// componentwise clamp into range [a,b]
	float2 clamp (float2 x, float2 a, float2 b);
	
	// componentwise clamp into range [0,1] also known as saturate in hlsl
	float2 clamp (float2 x);
	
	// get minimum component of vector, optionally get component index via min_index
	float min_component (float2 v, int* min_index=nullptr);
	
	// get maximum component of vector, optionally get component index via max_index
	float max_component (float2 v, int* max_index=nullptr);
	
	
	// componentwise floor
	float2 floor (float2 v);
	
	// componentwise ceil
	float2 ceil (float2 v);
	
	// componentwise round
	float2 round (float2 v);
	
	// componentwise floor to int
	int2 floori (float2 v);
	
	// componentwise ceil to int
	int2 ceili (float2 v);
	
	// componentwise round to int
	int2 roundi (float2 v);
	
	// componentwise pow
	float2 pow (float2 v, float2 e);
	
	// componentwise wrap
	float2 wrap (float2 v, float2 range);
	
	// componentwise wrap
	float2 wrap (float2 v, float2 a, float2 b);
	
	
	
	//// Angle conversion
	
	// converts degrees to radiants
	float2 to_radians (float2 deg);
	
	// converts radiants to degrees
	float2 to_degrees (float2 rad);
	
	// converts degrees to radiants
	// shortform to make degree literals more readable
	float2 deg (float2 deg);
	
	//// Linear interpolation
	
	// linear interpolation
	// like getting the output of a linear function
	// ex. t=0 -> a ; t=1 -> b ; t=0.5 -> (a+b)/2
	float2 lerp (float2 a, float2 b, float2 t);
	
	// linear mapping
	// sometimes called inverse linear interpolation
	// like getting the x for a y on a linear function
	// ex. map(70, 0,100) -> 0.7 ; map(0.5, -1,+1) -> 0.75
	float2 map (float2 x, float2 in_a, float2 in_b);
	
	// linear remapping
	// equivalent of lerp(out_a, out_b, map(x, in_a, in_b))
	float2 map (float2 x, float2 in_a, float2 in_b, float2 out_a, float2 out_b);
	
	
	//// Various interpolation
	
	// standard smoothstep interpolation
	float2 smoothstep (float2 x);
	
	// 3 point bezier interpolation
	float2 bezier (float2 a, float2 b, float2 c, float t);
	
	// 4 point bezier interpolation
	float2 bezier (float2 a, float2 b, float2 c, float2 d, float t);
	
	// 5 point bezier interpolation
	float2 bezier (float2 a, float2 b, float2 c, float2 d, float2 e, float t);
	
	
	//// Vector math
	
	// magnitude of vector
	float length (float2 v);
	
	// squared magnitude of vector, cheaper than length() because it avoids the sqrt(), some algorithms only need the squared magnitude
	float length_sqr (float2 v);
	
	// distance between points, equivalent to length(a - b)
	float distance (float2 a, float2 b);
	
	// normalize vector so that it has length() = 1, undefined for zero vector
	float2 normalize (float2 v);
	
	// normalize vector so that it has length() = 1, returns zero vector if vector was zero vector
	float2 normalizesafe (float2 v);
	
	// dot product
	float dot (float2 l, float2 r);
	
	// 2d cross product hack for convenient 2d stuff
	// same as cross({T.name[:-2]}3(l, 0), {T.name[:-2]}3(r, 0)).z,
	// ie. the cross product of the 2d vectors on the z=0 plane in 3d space and then return the z coord of that (signed mag of cross product)
	float cross (float2 l, float2 r);
	
	// rotate 2d vector counterclockwise 90 deg, ie. float2(-y, x) which is fast
	float2 rotate90 (float2 v);
	
}

