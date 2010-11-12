#ifndef COLLISION_H
#define COLLISION_H

#include "location.h"

class Collision {
public:
	static bool lineBoxRadius(Location B1, Location B2, Location L1, Location L2, const FixedPoint& = FixedPoint(0));
	static bool lineBox(Location B1, Location B2, Location L1, Location L2);
	static bool boxBox(Location, Location, Location, Location);
};

#endif

