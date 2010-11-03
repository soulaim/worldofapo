
#include "movable_object.h"

MovableObject::MovableObject():
	flags(0),
	dead(false)
{
}

void MovableObject::movableEnable(int enable_flags)
{
	flags |= enable_flags;
}

// TODO ALERT: Gravity is not a member of object, it is a member of world.
FixedPoint MovableObject::getGravity()
{
	return -FixedPoint(35,1000);
}

// Friction IS defined by the object. BUT: TODO: Friction is not constant over all objects.
FixedPoint MovableObject::getFriction()
{
	return FixedPoint(88,100);
}

void MovableObject::tick(const FixedPoint& terrainHeight)
{
	if(flags & AFFECTED_BY_GRAVITY_BIT)
		velocity.y += getGravity();

	if((flags & TERRAIN_COLLISION_BIT) && (terrainHeight > velocity.y + position.y))
	{
		position.y = terrainHeight;
		velocity.y = 0;
		
		if(flags & AFFECTED_BY_FRICTION_BIT)
		{
			FixedPoint friction = getFriction();
			velocity.x *= friction;
			velocity.z *= friction;
		}
	}
	
	position += velocity;
}

