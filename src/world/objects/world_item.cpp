#include "world/objects/world_item.h"
#include "world/objects/unit.h"
#include "world/world.h"

using std::stringstream;
using std::string;

WorldItem::WorldItem()
{
	type = OctreeObject::WORLD_ITEM;
	flags = AFFECTED_BY_GRAVITY_BIT | AFFECTED_BY_FRICTION_BIT | TERRAIN_COLLISION_BIT | OBJECT_COLLISION_BIT;
}

// todo, items to have different dimensions?
const Location& WorldItem::bb_top() const
{
    bb_top_.x = position.x + 1;
    bb_top_.y = position.y + 1;
    bb_top_.z = position.z + 1;
	return bb_top_;
}

const Location& WorldItem::bb_bot() const
{
    bb_bot_.x = position.x - 1;
    bb_bot_.y = position.y;
    bb_bot_.z = position.z - 1;
	return bb_bot_;
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

		if(u->id < 10000 && u->human())
		{
			// area transfer!
			if(intVals["AREA_CHANGE"])
			{
					u->intVals["NEEDS_AREA_TRANSFER"] = 1;
					u->strVals["NEXT_AREA"] = strVals["AREA_CHANGE_TARGET"];
			}
		}

		if(intVals["PERSISTS"] == 0)
			dead = true;
	}

	if(o.type == OctreeObject::WORLD_ITEM)
	{
		Location direction = (position - o.position);
		if(direction.length() == FixedPoint(0))
		{
			// unresolvable collision. leave it be.
			return;
		}

		direction.normalize();
		direction *= FixedPoint(1, 5);
		velocity += direction;
	}
}


string WorldItem::copyOrder(int ID) const
{
	stringstream item_msg;
	item_msg << "-2 ITEM " << ID << " " << type << " " << flags << " "
	<< position.x << " " << position.z << " " << position.y << " "
	<< velocity.x << " " << velocity.z << " " << velocity.y << " ";

	item_msg << HasProperties::copyOrder();
	item_msg << "#";

	return item_msg.str();
}

void WorldItem::handleCopyOrder(stringstream& ss)
{
	ss >> type >> flags >>
	position.x >> position.z >> position.y >>
	velocity.x >> velocity.z >> velocity.y;

	HasProperties::handleCopyOrder(ss);
}
