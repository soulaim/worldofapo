
#include "world/logic/unit_ai.h"
#include "world/world.h"
#include "world/level/level.h"
#include "world/objects/unit.h"
#include "misc/apomath.h"

#include <iostream>

using std::cout;
using std::endl;

void UnitAI::turnTowardsTarget(Unit& unit) {
    int best_angle;
    int best_upangle;

    Location direction;
    direction.x.setNumber(unit.intVals["DIRECTIONX"]);
    direction.z.setNumber(unit.intVals["DIRECTIONZ"]);
    direction.normalize();

    turnTowardsTarget(unit, direction, best_angle, best_upangle);
    unit.angle = best_angle;
    unit.upangle = best_upangle;
}

void UnitAI::turnTowardsTarget(Unit& me, Location& direction, int& best_angle, int& best_upangle) {

    Location myDirection;

	int hypo_angle = me.angle;
	int hypo_upangle = me.upangle;
	bool improved = true;
	FixedPoint best_error = FixedPoint(1000000);

	while(improved)
	{
		improved = false;
		hypo_angle += 5;
		myDirection.z = ApoMath::getSin(hypo_angle);
		myDirection.x = ApoMath::getCos(hypo_angle);

		FixedPoint error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
		if(error < best_error) {
			best_error = error;
			improved = true;
		}
		else {
			hypo_angle -= 10;
			myDirection.z = ApoMath::getSin(hypo_angle);
			myDirection.x = ApoMath::getCos(hypo_angle);

			error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else {
                hypo_angle += 5;
                hypo_angle += ApoMath::DEGREES_180;

                myDirection.z = ApoMath::getSin(hypo_angle);
                myDirection.x = ApoMath::getCos(hypo_angle);

                FixedPoint error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
                if(error < best_error) {
                    best_error = error;
                    improved = true;
                } else {
                    hypo_angle -= ApoMath::DEGREES_180;
                }
            }
		}
	}

	best_error = FixedPoint(1000000);
	improved = true;
	while(improved)
	{
		improved = false;
		hypo_upangle += 5;
		myDirection.y = ApoMath::getSin(hypo_upangle);
		FixedPoint error = (direction.y * 100 + myDirection.y * 100).squared();
		if(error < best_error)
		{
			best_error = error;
			improved = true;
		}
		else
		{
			hypo_upangle -= 10;
			myDirection.y = ApoMath::getSin(hypo_upangle);
			error = (direction.y * 100 + myDirection.y * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else
				hypo_upangle += 5;
		}
	}

	best_angle = hypo_angle;
	best_upangle = hypo_upangle;
}

void UnitAI::turnTowardsTarget(Unit& me, Unit& target, int& best_angle, int& best_upangle)
{
	// turn towards the target unit until facing it.
	Location direction = target.getEyePosition() - me.getEyePosition();

	if(direction.length() == 0)
	{
		// wtf, same position as my target? :G
		return;
	}

	direction.normalize();
    turnTowardsTarget(me, direction, best_angle, best_upangle);
}

bool UnitAI::roamingNextStepOk(World& world, Unit& unit) {

    const Location& pos = unit.getPosition();

    Location direction;
    direction.x.setNumber(unit.intVals["DIRECTIONX"]);
    direction.z.setNumber(unit.intVals["DIRECTIONZ"]);
    direction.normalize();

    return (world.lvl.getHeight(pos.x + direction.x * 4, pos.z + direction.z * 4) < FixedPoint(7));
}

Location UnitAI::findLongestLine(World& world, Unit& me) {
    const Location& myPos = me.getPosition();
    Level& level = world.lvl;
    ApoMath& math = world.apomath;

    Location best_direction;
    int best_value = -1;

    Location direction;

    int forbidden = me.intVals["DIRECTION_ID"] - 16;

    for(int i=0; i<32; ++i) {

        if(forbidden >= i - 8 && forbidden <= i + 8)
            continue;
        if(forbidden + 32 >= i - 8 && forbidden + 32 <= i + 8)
            continue;

        int cos = i * math.DEGREES_360 / 32;
        int sin = i * math.DEGREES_360 / 32;
        FixedPoint x = math.getCos(cos);
        FixedPoint z = math.getSin(sin);
        direction.x = x;
        direction.z = z;

        Location pos = myPos;

        for(int k=0; k<60; ++k) {
            if(k > best_value) {
                best_value = k;
                best_direction = direction;
                me.intVals["DIRECTION_ID"] = i;
            }

            pos += direction * 4;
            if(level.getHeight(pos.x, pos.z) > FixedPoint(7))
                break;
        }
    }

    return best_direction;
}


void UnitAI::tick(World& world, Unit& unit) {
    switch(unit.controllerTypeID)
	{
		case Unit::AI_ALIEN:
		{
            if(unit.intVals["STATE"] == 0) {
                cout << "ALIEN GOING ROAMING" << endl;
                unit.intVals["STATE"] = 1; // Roaming state.
                Location direction = findLongestLine(world, unit);
                unit.intVals["DIRECTIONX"] = direction.x.getAsInteger();
                unit.intVals["DIRECTIONZ"] = direction.z.getAsInteger();
                this->turnTowardsTarget(unit);
            }
            else if(unit.intVals["STATE"] == 1) {
                if(!roamingNextStepOk(world, unit)) {
                    Location direction = findLongestLine(world, unit);
                    unit.intVals["DIRECTIONX"] = direction.x.getAsInteger();
                    unit.intVals["DIRECTIONZ"] = direction.z.getAsInteger();
                    this->turnTowardsTarget(unit);
                }

                unit.keyState = Unit::MOVE_FRONT;

            }
			break;
		}

		case Unit::INANIMATE_OBJECT:
		{
			break;
		}

		default:
			break;
	}
}
