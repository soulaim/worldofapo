#ifndef MEDIKIT_H
#define MEDIKIT_H

#include <iostream>
#include <string>
#include <sstream>
#include "octree_object.h"
#include "hasproperties.h"

// item that is not being carried by anyone
class WorldItem: public OctreeObject, public HasProperties
{
public:
	WorldItem();
	Location bb_top() const;
	Location bb_bot() const;
	void collides(OctreeObject&);
	
	std::string copyOrder(int ID) const;
	void handleCopyOrder(std::stringstream& ss);
};

#endif
