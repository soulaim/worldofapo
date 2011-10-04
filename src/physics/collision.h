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

	static bool lineBoxRadius(const Location& B1, const Location& B2, Location L1, Location L2, const FixedPoint& = FixedPoint(0));
	static bool lineBox(const Location& B1, const Location& B2, Location L1, Location L2);
	static bool boxBox(const Location&, const Location&, const Location&, const Location&);
};

#endif

