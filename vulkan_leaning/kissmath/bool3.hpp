// file was generated by kissmath.py at <TODO: add github link>
#pragma once

namespace kissmath {
	//// forward declarations
	
	struct bool4;
	struct bool3;
	struct uint8v3;
	struct int64v3;
	struct float3;
	struct bool2;
	struct int3;
	
	struct bool3 {
		union { // Union with named members and array members to allow vector[] operator, not 100% sure that this is not undefined behavoir, but I think all compilers definitely don't screw up this use case
			struct {
				bool	x, y, z;
			};
			bool		arr[3];
		};
		
		// Component indexing operator
		bool& operator[] (int i);
		
		// Component indexing operator
		bool const& operator[] (int i) const;
		
		
		// uninitialized constructor
		bool3 ();
		
		// sets all components to one value
		// implicit constructor -> float3(x,y,z) * 5 will be turned into float3(x,y,z) * float3(5) by to compiler to be able to execute operator*(float3, float3), which is desirable
		// and short initialization like float3 a = 0; works
		bool3 (bool all);
		
		// supply all components
		bool3 (bool x, bool y, bool z);
		
		// extend vector
		bool3 (bool2 xy, bool z);
		
		// truncate vector
		bool3 (bool4 v);
		
		
		//// Truncating cast operators
		
		// truncating cast operator
		explicit operator bool2 () const;
		
		
		//// Type cast operators
		
		// type cast operator
		explicit operator uint8v3 () const;
		
		// type cast operator
		explicit operator int64v3 () const;
		
		// type cast operator
		explicit operator float3 () const;
		
		// type cast operator
		explicit operator int3 () const;
		
	};
	
	//// reducing ops
	
	// all components are true
	bool all (bool3 v);
	
	// any component is true
	bool any (bool3 v);
	
	
	//// boolean ops
	
	bool3 operator! (bool3 v);
	
	bool3 operator&& (bool3 l, bool3 r);
	
	bool3 operator|| (bool3 l, bool3 r);
	
	//// comparison ops
	
	// componentwise comparison returns a bool vector
	bool3 operator== (bool3 l, bool3 r);
	
	// componentwise comparison returns a bool vector
	bool3 operator!= (bool3 l, bool3 r);
	
	// vectors are equal, equivalent to all(l == r)
	bool equal (bool3 l, bool3 r);
	
	// componentwise ternary (c ? l : r)
	bool3 select (bool3 c, bool3 l, bool3 r);
	
}

