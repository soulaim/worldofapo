#ifndef COLLISION_H
#define COLLISION_H

#include "misc/vec3.h"

class Collision {
public:
	
	/*
	enum
	{
		NO_COLLISION = 0,
		
		X_POSITIVE,
		X_NEGATIVE,
		
		Y_POSITIVE,
		Y_NEGATIVE,
		
		Z_POSITIVE,
		Z_NEGATIVE
	};
	*/
	
	static bool lineBoxRadius(Location B1, Location B2, Location L1, Location L2, const FixedPoint& = FixedPoint(0));
	static bool lineBox(Location B1, Location B2, Location L1, Location L2);
	static bool boxBox(Location, Location, Location, Location);
};

#endif

