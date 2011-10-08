

#include "world/objects/items/tool.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

void ToolItemUsage::tick(WorldItem*, Unit&) {
}

void ToolItemUsage::onActivateReload(WorldItem*, World&, Unit&) {
}

void ToolItemUsage::onActivatePrimary(WorldItem*, World& world, Unit&) {
    world.add_message("^yToolItemUsage::onActivatePrimary ^wis ^rnot ^wimplemented");
}
