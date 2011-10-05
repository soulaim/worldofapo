
#include "world/objects/world_item.h"
#include "world/objects/unit.h"
#include "world/world.h"

using std::stringstream;
using std::string;


/*
int id = world.nextUnitID();
world.addProjectile(weapon_position, id, model_prototype);
Projectile& projectile = world.projectiles[id];

// need to move projectile out of self-range (don't want to shoot self LOL)
projectile_direction.normalize();
projectile.velocity = projectile_direction * FixedPoint(9, 2);
projectile.tick();
*/

void WorldItem::onActivate(World& world, Unit& unit) {
    world.add_message("^yWorldItem::onActivatePrimary ^wis ^rnot ^wimplemented");

    switch(intVals["TYPE"]) {
        case 0: // beam-weapon
            beamWeapon.onActivatePrimary(this, world, unit);
            break;
        case 1: // ballistic-weapon
            ballisticWeapon.onActivatePrimary(this, world, unit);
            break;
        case 2: // tool
            toolItem.onActivatePrimary(this, world, unit);
            break;
        case 3: // visual aid
            world.add_message("^yWorldItem::onActivatePrimary ^wis ^rnot ^wimplemented for visual aid");
            break;
        default:
            world.add_message("^yWorldItem::onActivatePrimary ^wis ^rnot ^wimplemented for type");
    }
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

