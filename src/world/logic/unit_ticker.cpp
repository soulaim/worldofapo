
#include "world/logic/unit_ticker.h"
#include "world/world.h"
#include "world/objects/unit.h"
#include "graphics/models/model.h"

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

	if(world.currentWorldFrame % 50 == 0)
		unit.regenerate();

    unit_ai.tick(world, unit);

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
		if(unit.velocity.y < FixedPoint(-7, 10))
			unit.soundInfo = "jump_land";

		unit.landingDamage();
		unit.applyFriction();

		if(unit.hasGroundUnderFeet()) {
			unit.position.y = world.lvl.getHeight(unit.position.x, unit.position.z);
			unit.velocity.y = FixedPoint::ZERO;
		}
	}

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

