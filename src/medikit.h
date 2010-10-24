#ifndef MEDIKIT_H
#define MEDIKIT_H

#include <iostream>
#include "octree_object.h"
#include "movable_object.h"
#include "location.h"

class Medikit: public OctreeObject, public MovableObject
{
public:
	Medikit();
	Location bb_top() const;
	Location bb_bot() const;
	void collides(OctreeObject&);
};

#endif
