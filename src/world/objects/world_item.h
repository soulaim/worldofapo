#ifndef MEDIKIT_H
#define MEDIKIT_H

#include <iostream>
#include <string>
#include <sstream>

#include "physics/octree_object.h"
#include "misc/hasproperties.h"

class World;
class Unit;

class WorldItem: public OctreeObject, public HasProperties
{
    // these are relevant only when the object is not wielded
    mutable Location bb_top_;
    mutable Location bb_bot_;

public:
	WorldItem();
	const Location& bb_top() const;
	const Location& bb_bot() const;
	void collides(OctreeObject&);

	std::string copyOrder(int ID) const;
	void handleCopyOrder(std::stringstream& ss);

    // these are relevant only when someone is wielding the item
    void onActivate(World& world, Unit& user);
	void onSecondaryActivate(World&, Unit&);

	bool onCollect(World&, Unit&);
	void onDrop(World&, Unit&);
	void tick(Unit& user);
};

#endif
