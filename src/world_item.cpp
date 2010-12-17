#include "world_item.h"
#include "unit.h"

WorldItem::WorldItem()
{
	type = OctreeObject::WORLD_ITEM;
	flags = AFFECTED_BY_GRAVITY_BIT | AFFECTED_BY_FRICTION_BIT | TERRAIN_COLLISION_BIT | OBJECT_COLLISION_BIT;
}

// todo, items to have different dimensions?
Location WorldItem::bb_top() const
{
	return Location(position.x+1, position.y+1, position.z+1);
}

Location WorldItem::bb_bot() const
{
	return Location(position.x-1, position.y, position.z-1);
}

void WorldItem::collides(OctreeObject& o)
{
	// dont collide more than once.
	if(dead)
		return;
	
	// TODO: better handling
	if(o.type == OctreeObject::UNIT)
	{
		Unit* u = static_cast<Unit*> (&o);
		
		// this is fucking stupid, but wtf..
		for(size_t i=0; i<u->weapons.size(); ++i)
		{
			u->intVals[u->weapons[i].strVals["AMMUNITION_TYPE"]] += 100 / u->weapons[i].intVals["AMMO_VALUE"];
		}
		
		dead = true;
	}
	
	if(o.type == OctreeObject::WORLD_ITEM)
	{
		// collision handling between items
	}
}

