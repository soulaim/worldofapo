
#ifndef H_MOVABLE_OBJECT
#define H_MOVABLE_OBJECT

#include "location.h"

class MovableObject
{
public:
	
	enum
	{
		AFFECTED_BY_GRAVITY_BIT = 1,
		AFFECTED_BY_FORCES_BIT = 16,
		TERRAIN_COLLISION_BIT = 2,
		OBJECT_COLLISION_BIT = 4,
		COLLISION_BOUNCE_BIT = 8
	};
	
	MovableObject();
	void movableEnable(int);
	void applyGravity();
	int tick(FixedPoint height);
	
	int flags;
	int dead;
	
	Location position;
	Location velocity;
};

#endif


