
#include "world/objects/items/ballistic.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/world_item.h"

#include <sstream>

/*
int id = world.nextUnitID();
world.addProjectile(weapon_position, id, model_prototype);
Projectile& projectile = world.projectiles[id];

// need to move projectile out of self-range (don't want to shoot self LOL)
projectile_direction.normalize();
projectile.velocity = projectile_direction * FixedPoint(9, 2);
projectile.tick();
*/

void BallisticWeaponUsage::getDetails(WorldItem* item, std::vector<std::string>& details) {
    std::stringstream ss_name;
    ss_name << "^YWeapon: ^R" << item->strVals["NAME"] << " ^w(^GBallistic^w)";

    std::stringstream ss_damage;
    ss_damage << "^YDamage per bullet: ^R" << item->intVals["DAMAGE"];

    std::stringstream ss_clip;
    ss_clip << "^YClip size: ^R" << item->intVals["CLIPSIZE"];

    std::stringstream ss_cooldown;
    ss_cooldown << "^YCooldown: ^R" << (item->intVals["COOLDOWN"] / 25.0f);

    std::stringstream ss_bullets;
    ss_bullets << "^YBullets per shot: ^R" << item->intVals["BPS"];

    std::stringstream ss_reload;
    ss_reload << "^YReload time: ^R" << (item->intVals["RELOAD_TIME"] / 25.0f);

    std::stringstream ss_dps;
    ss_dps << "^YTotal DPS: ^R" << (item->intVals["DAMAGE"] * item->intVals["BPS"] / (item->intVals["COOLDOWN"] / 25.0f));

    details.push_back(ss_name.str());
    details.push_back(ss_clip.str());
    details.push_back(ss_cooldown.str());
    details.push_back(ss_reload.str());
    details.push_back(ss_bullets.str());
    details.push_back(ss_damage.str());
    details.push_back(ss_dps.str());
}

void BallisticWeaponUsage::tick(WorldItem* item, Unit&) {
    if(item->intVals["RELOADING"]) {
        if(--item->intVals["RLTIME"] == 0)
            item->intVals["RELOADING"] = 0;
    } else if(item->intVals["CD"] > 0) {
        --item->intVals["CD"];
    }
}

void BallisticWeaponUsage::onActivateReload(WorldItem* item, World& world, Unit&) {
    if(item->intVals["RELOADING"])
        return;

    world.add_message("RELOAD");
    item->intVals["RELOADING"] = 1;
    item->intVals["RLTIME"] = item->intVals["RELOAD_TIME"];
    item->intVals["CLIP"] = item->intVals["CLIPSIZE"];
}

void BallisticWeaponUsage::onActivatePrimary(WorldItem* item, World& world, Unit& caster) {

    if(item->intVals["RELOADING"])
        return;

    if(item->intVals["CD"] > 0)
        return;

    if(item->intVals["CLIP"] == 0) {
        onActivateReload(item, world, caster);
        return;
    }

    world.add_message("SHOOT!");

    item->intVals["CD"] = item->intVals["COOLDOWN"];
    --item->intVals["CLIP"];

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

    p["DAMAGE"] = 100;
    p["BALLISTIC"] = 1;
    p["TPF"] = 15;
    p["DEATH_IF_HITS_UNIT"] = 1;
    p["LIFETIME"] = 5;
    p["MASS"] = 50;
}
