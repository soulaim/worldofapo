
#include "world/logic/item_ticker.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "graphics/models/model.h"

#include <cassert>

void ItemTicker::tickItem(World& world, WorldItem& item, Model* model)
{
	// wut
	if(world.visualworld->isActive())
	{
		assert(model && "item model does not exist");
		model->updatePosition(item.position.x.getFloat(), item.position.y.getFloat(), item.position.z.getFloat());
	}

	if(item.intVals["AREA_CHANGE"])
	{
		world.visualworld->genParticleEmitter(item.position + Location(0, 1, 0), item.velocity + Location(0, 1, 0),
										3, 1000, 1000, "GREEN", "GREEN", "GREEN", "GREEN", 1000, 5, 100);
	}

    // gravity
	item.velocity.y -= FixedPoint(120,1000);

	// some physics & game world information
	if( (item.velocity.y + item.position.y - FixedPoint(1, 20)) <= world.lvl.getHeight(item.position.x, item.position.z) )
	{
		// colliding with terrain right now
		FixedPoint friction = FixedPoint(88, 100);

		item.position.y = world.lvl.getHeight(item.position.x, item.position.z);

		item.velocity.y = FixedPoint(50, 1000); // no clue if this makes any sense
		item.velocity.x *= friction;
		item.velocity.z *= friction;
	}

	auto& potColl = world.octree->nearObjects(item.position);

	for(auto iter = potColl.begin(); iter != potColl.end(); ++iter)
	{
		// handle only unit collisions
		if ((*iter)->type != OctreeObject::UNIT)
			continue;

		Unit* u = static_cast<Unit*>(*iter);

		// now did they collide or not?
		if( (item.position - u->position).lengthSquared() < FixedPoint(16) ) {
			item.collides(*u);
		}
	}

	item.position += item.velocity;
	world.lvl.clampToLevelArea(item);
}

