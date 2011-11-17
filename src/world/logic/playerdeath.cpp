
#include "world/logic/playerdeath.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include <sstream>

void PlayerDeathHandler::handle(World& world, Unit& unit) {

    std::stringstream ss; ss << "^G" << unit.name << " ^Rhas died!";
    world.add_message(ss.str());

    /*
    unit["HEALTH"] = unit.getMaxHP();
    Location pos = world.lvl.getStartLocation();
    unit.setPosition(pos);
    unit.zeroMovement();
    */
}
