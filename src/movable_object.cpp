
#include "movable_object.h"

MovableObject::MovableObject():
	flags(0),
	dead(false),
	angle(0),
	upangle(0)
{
}

void MovableObject::movableEnable(int enable_flags)
{
	flags |= enable_flags;
}

const Location& MovableObject::getPosition()
{
	return position;
}

const Location& MovableObject::getLookDirection()
{
	static ApoMath apomath;
	apomath.init(3000);
	
	FixedPoint cos = apomath.getCos(angle);
	FixedPoint sin = apomath.getSin(angle);
	FixedPoint upcos = apomath.getCos(upangle);
	FixedPoint upsin = apomath.getSin(upangle);
	
	FixedPoint x = 30; // what is this actually? :D
	
	lookDirection.x = cos * upcos * x;
	lookDirection.z = sin * upcos * x;
	lookDirection.y =      -upsin * x;
	return lookDirection;
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

