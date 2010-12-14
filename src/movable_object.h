
#ifndef H_MOVABLE_OBJECT
#define H_MOVABLE_OBJECT

#include "location.h"

class MovableObject
{
	Location lookDirection;
public:
	
	enum
	{
		AFFECTED_BY_GRAVITY_BIT = 1,
		AFFECTED_BY_FORCES_BIT = 16,
		AFFECTED_BY_FRICTION_BIT = 32,
		TERRAIN_COLLISION_BIT = 2,
		OBJECT_COLLISION_BIT = 4,
		COLLISION_BOUNCE_BIT = 8
	};
	
	MovableObject();
	
	virtual FixedPoint getGravity() const;
	virtual FixedPoint getFriction() const;
	
	void movableEnable(int);
	void tick(const FixedPoint& height);
	
	const Location& getLookDirection();
	const Location& getPosition() const;
	
	int flags;
	int dead;
	
	int angle;
	int upangle;
	
	Location position;
	Location prev_position;
	Location velocity;
};

#endif

