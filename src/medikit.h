#ifndef MEDIKIT_H
#define MEDIKIT_H

#include <iostream>
#include "octree_object.h"

class Medikit: public OctreeObject
{
	friend class World; // hohhoijaa
public:
	Medikit();
	Location bb_top() const;
	Location bb_bot() const;
	void collides(OctreeObject&);
};

#endif
