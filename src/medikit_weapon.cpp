#include "medikit_weapon.h"

#include "unit.h"
#include "world.h"

void MedikitWeapon::fire() {
	if(cooldown_left > 0)
	{
		--cooldown_left;
		return;
	}

	u.soundInfo = "shoot";

	Location pos;
	pos.x = 30;
	pos.z = 0;
	pos.y = 0;

	int angle   = u.angle;
	int upangle = u.upangle;

	FixedPoint cos = w.apomath.getCos(angle);
	FixedPoint sin = w.apomath.getSin(angle);
	FixedPoint upcos = w.apomath.getCos(upangle);
	FixedPoint upsin = w.apomath.getSin(upangle);
	
	Location relative_pos;
	FixedPoint x = pos.x;
	FixedPoint y = pos.y;
	FixedPoint z = pos.z;
	
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
	kit.position += kit_direction * FixedPoint(2);
	kit.velocity = kit_direction * FixedPoint(1) + u.velocity;
	
	cooldown_left = cooldown;
}

void MedikitWeapon::tick() {
	if (cooldown_left > 0)
		cooldown_left -= 1;
}

