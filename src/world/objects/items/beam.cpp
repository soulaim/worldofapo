

#include "world/objects/items/beam.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

#include <sstream>

void BeamWeaponUsage::getDetails(WorldItem* item, std::vector<std::string>& details) {
    std::stringstream ss_name;
    ss_name << item->strVals["NAME"] << " ^g(Beam)";

    details.push_back(ss_name.str());
}

void BeamWeaponUsage::tick(WorldItem*, Unit&) {
}

void BeamWeaponUsage::onActivateReload(WorldItem*, World& world, Unit&) {
    world.add_message("^yBeamWeaponUsage::onActivateReload ^wis ^rnot ^wimplemented");
}

void BeamWeaponUsage::onActivatePrimary(WorldItem*, World& world, Unit&) {
    world.add_message("^yBeamWeaponUsage::onActivatePrimary ^wis ^rnot ^wimplemented");
}

