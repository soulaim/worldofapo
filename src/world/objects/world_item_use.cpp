
#include "world/objects/world_item.h"
#include "world/objects/unit.h"
#include "world/world.h"

using std::stringstream;
using std::string;




void WorldItem::onActivate(World& world, Unit& unit) {

    switch(intVals["TYPE"]) {
        case 0: // beam-weapon
            beamWeapon.onActivatePrimary(this, world, unit);
            break;
        case 1: // ballistic-weapon
            ballisticWeapon.onActivatePrimary(this, world, unit);
            break;
        case 2: // tool
            toolItem.onActivatePrimary(this, world, unit);
            break;
        case 3: // visual aid
            world.add_message("^yWorldItem::onActivatePrimary ^wis ^rnot ^wimplemented for visual aid");
            break;
        case 4: // consumable
            world.add_message("^yWorldItem::onActivatePrimary ^wis ^rnot ^wimplemented for consumables");
            break;
        default:
            world.add_message("^yWorldItem::onActivatePrimary ^wis ^rnot ^wimplemented for type");
    }
}

void WorldItem::onSecondaryActivate(World& world, Unit&) {
    world.add_message("^yWorldItem::onActivateSecondary ^wis ^rnot ^wimplemented");
}


bool WorldItem::onCollect(World& world, Unit&) {
    world.add_message("^yWorldItem::onCollect ^wis ^rnot ^wimplemented");
    return false;
}

void WorldItem::onDrop(World& world, Unit&) {
    world.add_message("^yWorldItem::onDrop ^wis ^rnot ^wimplemented");
}

void WorldItem::tick(Unit& unit) {
    switch(intVals["TYPE"]) {
        case 0: // beam-weapon
            beamWeapon.tick(this, unit);
            break;
        case 1: // ballistic-weapon
            ballisticWeapon.tick(this, unit);
            break;
        case 2: // tool
            toolItem.tick(this, unit);
            break;
        case 3: // visual aid
            break;
        case 4: // consumable
            break;
        default:
            ;
    }
}

