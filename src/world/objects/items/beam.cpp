

#include "world/objects/items/beamweaponusage.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

void BeamWeaponUsage::onActivatePrimary(WorldItem*, World& world, Unit&) {
    world.add_message("^yBeamWeaponUsage::onActivatePrimary ^wis ^rnot ^wimplemented");
}

