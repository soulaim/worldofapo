
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


bool UnitAI::isLineClear(World& world, const Location& pos1, const Location& pos2) {
    Location current_pos = pos1;
    Location direction = pos2 - pos1;
    direction /= 20;

    for(int i=0; i<20; ++i) {
        current_pos += direction;
        if(world.lvl.getHeight(current_pos.x, current_pos.z) > 6) {
            return false;
        }
    }
    return true;
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
        unit.keyState = 0;
        unit.mouseButtons = 0;

		case Unit::AI_ALIEN:
		{
            int stateValue = unit.intVals["STATE"];

            // if not doing anything
            if(stateValue == 0) {
                unit.intVals["STATE"] = 1; // Roaming state.
                Location direction = findLongestLine(world, unit);
                unit.intVals["DIRECTIONX"] = direction.x.getAsInteger();
                unit.intVals["DIRECTIONZ"] = direction.z.getAsInteger();
                this->turnTowardsTarget(unit);
            }
            else if(stateValue == 1) {
                if(!roamingNextStepOk(world, unit)) {
                    Location direction = findLongestLine(world, unit);
                    unit.intVals["DIRECTIONX"] = direction.x.getAsInteger();
                    unit.intVals["DIRECTIONZ"] = direction.z.getAsInteger();
                    this->turnTowardsTarget(unit);
                }
                unit.keyState = Unit::MOVE_FRONT;

                // heartbeat - check if creep notices enemies
                if((unit.birthTime + world.currentWorldFrame + unit.id * 631) % 50 == 0) {
                    for(std::map<int, Unit>::iterator it = world.units.begin(); it != world.units.end(); ++it) {
                        if(it->first >= 10000)
                            break;
                        if(this->isLineClear(world, unit.getPosition(), it->second.getPosition())) {
                            unit.intVals["STATE"] = 2;
                            unit.intVals["T"] = it->first;
                        }
                    }
                }
            }
            else if(stateValue == 2) {

                // hearbeat, pick nearest target
                if((unit.birthTime + world.currentWorldFrame + unit.id * 631) % 50 == 0) {
                    int current_target = -1;
                    FixedPoint distance = FixedPoint(1000);
                    for(std::map<int, Unit>::iterator it = world.units.begin(); it != world.units.end(); ++it) {
                        if(it->first >= 10000)
                            break;

                        FixedPoint tmp_dist = (it->second.getPosition() - unit.getPosition()).lengthSquared();
                        if( tmp_dist < distance ) {
                            current_target = it->second.id;
                            distance = tmp_dist;
                        }
                    }

                    unit["T"] = current_target;
                }

                std::map<int, Unit>::iterator it = world.units.find(unit["T"]);
                if(unit["T"] != -1 && (it != world.units.end()) ) {
                    int angle = unit.angle;
                    int upangle = unit.upangle;
                    this->turnTowardsTarget(unit, it->second, angle, upangle);
                    unit.angle = angle;
                    unit.upangle = upangle;
                    FixedPoint dist = (unit.getPosition() - it->second.getPosition()).lengthSquared();

                    // this should be ranged behaviour
                    if( dist < 1000 ) {
                        unit.keyState |= Unit::MOVE_LEFT;
                    }
                    else {
                        unit.keyState |= Unit::MOVE_FRONT;
                    }
                    unit.mouseButtons |= Unit::MOUSE_LEFT;

                    // this should be melee behaviour
                    if(dist < 100) {
                        it->second.takeDamage(1);
                        it->second.last_damage_dealt_by = unit.id;
                        it->second("DAMAGED_BY") = "devour";
                        // TODO: sound + particles
                    }
                    if((unit.birthTime * 7 + world.currentWorldFrame + unit.id * 13) % 50 == 0) {
                        if(unit.hasSupportUnderFeet()) {
                            unit.velocity.y += FixedPoint(15, 15);
                            unit.velocity += unit.getLookDirection() * FixedPoint(13, 20);
                        }
                    }
                }

                // heartbeat - if can't see enemies and enemies are not nearby, go roaming.
                if( (unit.birthTime + world.currentWorldFrame + unit.id * 631) % 50 == 0 ) {
                    if(it == world.units.end()) {
                        unit.intVals["STATE"] = 1;
                    }
                    else if(!isLineClear(world, unit.getPosition(), it->second.getPosition())) {
                        unit.intVals["STATE"] = 1;
                    }
                }

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
