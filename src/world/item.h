
#ifndef H_ITEM
#define H_ITEM

class World;
class Unit;

#include "physics/octree_object.h"
#include "misc/hasproperties.h"

// TODO: Items are not octree objects all the time?
class Item : public HasProperties
{
public:
	virtual void onActivate(World&, Unit&) = 0;
	virtual void onSecondaryActivate(World&, Unit&) = 0;
	
	virtual bool onCollect(World&, Unit&) = 0; // returns true if collecting was successful. false otherwise.
	virtual void onDrop(World&, Unit&) = 0;
	virtual void onConsume(World&, Unit&) = 0;
};

#endif
