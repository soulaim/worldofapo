
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

FixedPoint MovableObject::getGravity()
{
	return -FixedPoint(35,1000);
}

FixedPoint MovableObject::getFriction()
{
	return FixedPoint(88,100);
}

void MovableObject::tick(FixedPoint terrainHeight)
{
	if (flags & AFFECTED_BY_GRAVITY_BIT)
		velocity.y += getGravity();

	if ((flags & TERRAIN_COLLISION_BIT) && (terrainHeight > velocity.y + position.y))
	{
		position.y = terrainHeight;
		velocity.y = 0;
		if (flags & AFFECTED_BY_FRICTION_BIT)
		{
			FixedPoint friction = getFriction();
			velocity.x *= friction;
			velocity.z *= friction;
		}
	}
	position += velocity;
}

