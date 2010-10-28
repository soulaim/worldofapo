#include "medikit_weapon.h"

#include "unit.h"
#include "world.h"

void MedikitWeapon::fire() {
	if(cooldown_left > 0)
	{
		--cooldown_left;
		return;
	}

	int angle   = u.angle;
	int upangle = u.upangle;

	FixedPoint cos = w.apomath.getCos(angle);
	FixedPoint sin = w.apomath.getSin(angle);
	FixedPoint upcos = w.apomath.getCos(upangle);
	FixedPoint upsin = w.apomath.getSin(upangle);
	
	Location relative_pos;
	FixedPoint x = 30;
	FixedPoint y = 0;
	FixedPoint z = 0;
	
	relative_pos.x = cos * upcos * x - sin * z + cos * upsin * y;
	relative_pos.z = sin * upcos * x + cos * z + sin * upsin * y;
	relative_pos.y =      -upsin * x           +       upcos * y;

	Location weapon_pos = u.position;
	Location kit_direction = relative_pos;
	
	weapon_pos.y += 4;
	kit_direction.y += 4;

	int id = w.nextUnitID();
	w.medikits[id].position = weapon_pos;
	
	Medikit& kit = w.medikits[id];
	
	kit_direction.normalize();
	kit.position += kit_direction * FixedPoint(3);
	kit.velocity = kit_direction * FixedPoint(1,4) + u.velocity;
	
	cooldown_left = cooldown;
}

void MedikitWeapon::tick() {
	if (cooldown_left > 0)
		cooldown_left -= 1;
}

FixedPoint MedikitWeapon::getFriction() {
	return FixedPoint(1);
}

