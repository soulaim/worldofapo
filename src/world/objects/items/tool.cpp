

#include "world/objects/items/toolitemusage.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

void ToolItemUsage::onActivatePrimary(WorldItem*, World& world, Unit&) {
    world.add_message("^yToolItemUsage::onActivatePrimary ^wis ^rnot ^wimplemented");
}

