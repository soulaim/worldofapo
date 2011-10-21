
#include "world/logic/playerdeath.h"
#include "world/world.h"
#include "world/objects/unit.h"

void PlayerDeathHandler::handle(World& world, Unit& unit) {
    unit["HEALTH"] = unit.getMaxHP();

    Location pos = world.lvl.getStartLocation();
    unit.setPosition(pos);
    unit.zeroMovement();
}
