#ifndef MEDIKIT_H
#define MEDIKIT_H

#include <vector>
#include <string>
#include <sstream>

#include "physics/octree_object.h"
#include "misc/hasproperties.h"

#include "world/objects/items/ballistic.h"
#include "world/objects/items/beam.h"
#include "world/objects/items/tool.h"
#include "world/objects/items/armor.h"
#include "world/objects/items/small.h"

class World;
class Unit;

class WorldItem: public OctreeObject, public HasProperties
{
    // these are relevant only when the object is not wielded
    mutable Location bb_top_;
    mutable Location bb_bot_;

    BeamWeaponUsage beamWeapon;
    BallisticWeaponUsage ballisticWeapon;
    ToolItemUsage toolItem;
    ArmorItemUsage armorItem;
    SmallItemUsage smallItem;

public:
	WorldItem();
	const Location& bb_top() const;
	const Location& bb_bot() const;
	void collides(OctreeObject&);

	std::string copyOrder(int ID) const;
	void handleCopyOrder(std::stringstream& ss);

    std::string inventoryCopy() const;
    void handleInventoryCopy(std::stringstream& ss);

    // for displaying item information on screen.
    void getDetails(std::vector<std::string>&);

    // these are relevant only when someone is wielding the item
    void onActivate(World& world, Unit& user);
	void onSecondaryActivate(World&, Unit&);
    void onReload(World&, Unit&);

	bool onCollect(World&, Unit&);
	void onDrop(World&, Unit&);
	void tick(Unit& user);
};

#endif
