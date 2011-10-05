

#include "world/objects/items/beam.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

void BeamWeaponUsage::tick(WorldItem*, Unit&) {
}

void BeamWeaponUsage::onActivatePrimary(WorldItem*, World& world, Unit&) {
    world.add_message("^yBeamWeaponUsage::onActivatePrimary ^wis ^rnot ^wimplemented");
}

