
#include "octree_object.h"

OctreeObject::OctreeObject():
	type(UNDEFINED),
	id(-1),
	collision_rule(CollisionRule::HARD_OBJECT),
	staticObject(0)
{
}

