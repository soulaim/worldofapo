
#include "world/objects/items/ballisticweaponusage.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

void BallisticWeaponUsage::onActivatePrimary(WorldItem*, World& world, Unit&) {
    world.add_message("^yBallisticWeaponUsage::onActivatePrimary ^wis ^rnot ^wimplemented");
}
