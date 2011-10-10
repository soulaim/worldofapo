

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

void BeamWeaponUsage::onActivatePrimary(WorldItem* item, World& world, Unit& caster) {
    if(item->intVals["POWER"] < item->intVals["DRAIN"])
        return;
    item->intVals["POWER"] -= item->intVals["DRAIN"];


    Location eyePos = caster.getEyePosition();

    int id = world.nextUnitID();
    world.addProjectile(eyePos, id, VisualWorld::INVISIBLE_MODEL);
    Projectile& p = world.projectiles[id];

    Location direction = caster.getLookDirection();
    caster.velocity -= direction * FixedPoint(1, 100);

    p.velocity = direction;
    p.velocity *= FixedPoint(9, 2);
    p.tick();
    p.velocity *= FixedPoint(2, 9);

    p["OWNER"] = caster.id;
    p("NAME") = (*item)("NAME");

    p["DAMAGE"] = item->intVals["DAMAGE"];
    p["BEAM"] = 1;
    p["TPF"] = 10;
    p["DEATH_IF_HITS_UNIT"] = 1;
    p["LIFETIME"] = 10;

}

