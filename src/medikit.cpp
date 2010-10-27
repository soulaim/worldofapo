#include "medikit.h"

Medikit::Medikit()
{
	type = OctreeObject::MEDIKIT;
	flags = 0;
	flags |= AFFECTED_BY_GRAVITY_BIT;
	flags |= AFFECTED_BY_FRICTION_BIT;
	flags |= TERRAIN_COLLISION_BIT;
	flags |= OBJECT_COLLISION_BIT;
}

Location Medikit::bb_top() const
{
	return Location(position.x+1, position.y+1, position.z+1);
}

Location Medikit::bb_bot() const
{
	return Location(position.x-1, position.y, position.z-1);
}

void Medikit::collides(OctreeObject& o) {
	if (o.type == OctreeObject::UNIT)
		dead = true;
}

