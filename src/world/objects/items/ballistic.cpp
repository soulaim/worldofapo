
#include "world/objects/items/ballistic.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

/*
int id = world.nextUnitID();
world.addProjectile(weapon_position, id, model_prototype);
Projectile& projectile = world.projectiles[id];

// need to move projectile out of self-range (don't want to shoot self LOL)
projectile_direction.normalize();
projectile.velocity = projectile_direction * FixedPoint(9, 2);
projectile.tick();
*/

void BallisticWeaponUsage::tick(WorldItem* item, Unit&) {
    if(item->intVals["RELOADING"]) {
        if(--item->intVals["RLTIME"] == 0)
            item->intVals["RELOADING"] = 0;
    } else if(item->intVals["CD"] > 0) {
        --item->intVals["CD"];
    }
}

void BallisticWeaponUsage::onActivatePrimary(WorldItem* item, World& world, Unit& caster) {

    if(item->intVals["RELOADING"])
        return;

    if(item->intVals["CD"] > 0)
        return;

    if(item->intVals["CLIP"] == 0) {
        world.add_message("RELOAD");
        item->intVals["RELOADING"] = 1;
        item->intVals["RLTIME"] = item->intVals["RELOAD_TIME"];
        item->intVals["CLIP"] = item->intVals["CLIPSIZE"];
        return;
    }

    world.add_message("SHOOT!");
    item->intVals["CD"] = item->intVals["COOLDOWN"];

    int id = world.nextUnitID();

    Location eyePos = caster.getEyePosition();
    world.addProjectile(eyePos, id, VisualWorld::INVISIBLE_MODEL);
    Projectile& p = world.projectiles[id];

    Location direction = caster.getLookDirection();
    caster.velocity -= direction * FixedPoint(5, 100);

    p.velocity = direction;
    p.velocity.normalize();
    p.velocity *= FixedPoint(9, 2);
    p.tick();
    p.velocity *= FixedPoint(2, 9);

    p["OWNER"] = caster.id;
    p("NAME") = (*item)("NAME");

    p["TPF"] = 15;
    p["DEATH_IF_HITS_UNIT"] = 1;
    p["LIFETIME"] = 5;
}
