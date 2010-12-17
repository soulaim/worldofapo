#ifndef MEDIKIT_H
#define MEDIKIT_H

#include <iostream>
#include "octree_object.h"

// item that is not being carried by anyone
class WorldItem: public OctreeObject
{
public:
	WorldItem();
	Location bb_top() const;
	Location bb_bot() const;
	void collides(OctreeObject&);
};

#endif
