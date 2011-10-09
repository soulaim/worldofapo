

#include "world/objects/items/tool.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

void ToolItemUsage::getDetails(WorldItem* item, std::vector<std::string>& details) {
    std::stringstream ss_name;
    ss_name << item->strVals["NAME"] << " ^g(Tool)";

    details.push_back(ss_name.str());
}

void ToolItemUsage::tick(WorldItem*, Unit&) {
}

void ToolItemUsage::onActivateReload(WorldItem*, World&, Unit&) {
}

void ToolItemUsage::onActivatePrimary(WorldItem*, World& world, Unit&) {
    world.add_message("^yToolItemUsage::onActivatePrimary ^wis ^rnot ^wimplemented");
}
