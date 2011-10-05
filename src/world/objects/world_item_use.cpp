
#include "world/objects/world_item.h"
#include "world/objects/unit.h"
#include "world/world.h"

using std::stringstream;
using std::string;


void WorldItem::onActivate(World& world, Unit&) {
    world.add_message("^yWorldItem::onActivatePrimary ^wis ^rnot ^wimplemented");
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

void WorldItem::tick(Unit&) {

}