#ifndef MEDIKIT_H
#define MEDIKIT_H

#include <iostream>
#include "octree_object.h"
#include "location.h"

class Medikit: public OctreeObject
{
public:
	Medikit()
	{
		type = OctreeObject::MEDIKIT;
		pos = Location(5, 5, 5);
	};

	Location pos;
	Location bb_top();
	Location bb_bot();
	void collides(OctreeObject&);
};

#endif
