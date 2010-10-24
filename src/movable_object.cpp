
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

void MovableObject::applyGravity()
{
	velocity.y -= FixedPoint(35,1000);
}

int MovableObject::tick(FixedPoint height)
{
	if (dead)
		return true;

	if (flags & AFFECTED_BY_GRAVITY_BIT)
		applyGravity();

	if ((flags & TERRAIN_COLLISION_BIT) && (height > velocity.y + position.y))
	{
		position.y = height;
		velocity.y = 0;
	}
	position += velocity;

	return false;
}

