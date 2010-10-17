#ifndef COLLISION_H
#define COLLISION_H

#include "location.h"

class Collision {
	static int GetIntersection(FixedPoint fDst1, FixedPoint fDst2, Location P1, Location P2, Location &Hit);
	static int InBox(Location Hit, Location B1, Location B2, const int Axis);
public:
	static bool lineBox(Location B1, Location B2, Location L1, Location L2);
};

#endif

