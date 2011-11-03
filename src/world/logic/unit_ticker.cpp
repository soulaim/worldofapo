
#include "world/logic/unit_ticker.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "graphics/models/model.h"

#include "misc/messaging_system.h"

FixedPoint heightDifference2Velocity(const FixedPoint& h_diff)
{
	// no restrictions for moving downhill
	if(h_diff < FixedPoint::ZERO)
		return 1;
	if(h_diff >= FixedPoint(2))
		return FixedPoint::ZERO;
	return (FixedPoint(2) - h_diff)/FixedPoint(2);
}

void UnitTicker::tickUnit(World& world, Unit& unit, Model* model)
{
	unit.intVals["D"] *= 0.95;

    if(unit.human()) {
        int zen_modifier = unit.getModifier("ZEN");
        int& sanity = unit["SANITY"];

        // sanity tremble
        if(sanity < 60) {
            RandomMachine random;
            random.setSeed(world.currentWorldFrame);
            int maxEffect = (60 - sanity < 30) ? (60 - sanity) : 30;
            int mouse_effect1 = 3 * (random.getInt() % (maxEffect)) * ((random.getInt() & 1) * 2 - 1);
            int mouse_effect2 = 3 * (random.getInt() % (maxEffect)) * ((random.getInt() & 1) * 2 - 1);
            unit.angle += mouse_effect1;
            unit.upangle += mouse_effect2;
        }

        if(world.currentWorldFrame % (10 * zen_modifier) == 0) {
            --sanity;
            if(sanity < 0) {
                sanity = 0;
                unit.last_damage_dealt_by = unit.id;
                unit.takeDamage(20, DamageType::PURE);
                unit("DAMAGED_BY") = "depression";
            }
        }
    }

	if(world.currentWorldFrame % 50 == 0)
		unit.regenerate();

	// for server it's ok that there are no models sometimes :G
	if(world.visualworld->isActive())
	{
		assert(model && "this should never happen");
		model->rotate_y(unit.getAngle());
		model->updatePosition(unit.position.x.getFloat(), unit.position.y.getFloat(), unit.position.z.getFloat());
	}

	// some physics & game world information
	if( (unit.velocity.y + unit.position.y - FixedPoint(1, 20)) <= world.lvl.getHeight(unit.position.x, unit.position.z) )
		unit.mobility |= Unit::MOBILITY_STANDING_ON_GROUND;

	if(unit.hasSupportUnderFeet())
	{
		if(unit.velocity.y < FixedPoint(-7, 10)) {
            sendMsg(SoundEvent("jump_land", 100000, unit.getEyePosition()));
        }

		unit.landingDamage();
		unit.applyFriction();

		if(unit.hasGroundUnderFeet()) {
			unit.position.y = world.lvl.getHeight(unit.position.x, unit.position.z);
			unit.velocity.y = FixedPoint::ZERO;
		}
	}

    unit_ai.tick(world, unit);

    Location tmp;
    for(int i=0; i<world.apomath.DEGREES_360; i+=world.apomath.DEGREES_360/32) {
        FixedPoint& tmp_x = world.apomath.getCos(i);
        FixedPoint& tmp_z = world.apomath.getSin(i);
        if(world.lvl.getHeight(unit.position.x + tmp_x, unit.position.z + tmp_z) > 8) {
            unit.position += Location(-tmp_x * FixedPoint(2, 20), 0, -tmp_z * FixedPoint(2, 20));
        }
    }

	unit.applyGravity();
	unit.updateMobility();
	unit.processInput(world);

	// weapon activations
	if(unit.getMouseAction(Unit::MOUSE_LEFT)) {
        unit.activateCurrentItemPrimary(world);
	}

	if(unit.getMouseAction(Unit::MOUSE_RIGHT)) {
        unit.activateCurrentItemSecondary(world);
	}

    if(unit.getKeyAction(Unit::RELOAD)) {
        unit.activateCurrentItemReload(world);
    }

    if(unit.getKeyAction(Unit::INTERACT)) {
        WorldItem* item = unit.itemPick.get();
        if((item != 0) && (item->dead == 0)) {
            unit.inventory.pickUp(world, unit, item);
        }
    }


	FixedPoint reference_x = unit.position.x + unit.velocity.x;
	FixedPoint reference_z = unit.position.z + unit.velocity.z;
	FixedPoint reference_y = world.lvl.getHeight(reference_x, reference_z);
	FixedPoint y_diff = reference_y - unit.position.y;
	FixedPoint yy_val = heightDifference2Velocity(y_diff);

	unit.tick(yy_val);
	world.lvl.clampToLevelArea(unit);
	unit.postTick();
}

