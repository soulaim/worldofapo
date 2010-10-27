#include "machinegun.h"
#include "world.h"
#include "unit.h"

void MachineGun::fire()
{
	if(cooldown_left > 0)
	{
		--cooldown_left;
		return;
	}

	u.soundInfo = "shoot";

	Location position;
	position.x = 30;
	position.z = 0;
	position.y = 0;

	int angle   = u.angle;
	int upangle = u.upangle;

	FixedPoint cos = w.apomath.getCos(angle);
	FixedPoint sin = w.apomath.getSin(angle);
	FixedPoint upcos = w.apomath.getCos(upangle);
	FixedPoint upsin = w.apomath.getSin(upangle);
	
	Location relative_position;
	FixedPoint x = position.x;
	FixedPoint y = position.y;
	FixedPoint z = position.z;
	
	relative_position.x = cos * upcos * x - sin * z + cos * upsin * y;
	relative_position.z = sin * upcos * x + cos * z + sin * upsin * y;
	relative_position.y =      -upsin * x           +       upcos * y;

	Location weapon_position = u.position;
	Location projectile_direction = relative_position;
	
	weapon_position.y += 4;
	projectile_direction.y += 4;

	int id = w.nextUnitID();
	w.addProjectile(weapon_position, id);
	
	Projectile& projectile = w.projectiles[id];
	
	projectile_direction.normalize();
	projectile.velocity = projectile_direction * FixedPoint(45, 10) + u.velocity;
	projectile.tick(); // need to move projectile out of self-range (don't want to shoot self LOL)
	
	projectile.velocity = projectile_direction * FixedPoint(10, 1) + u.velocity;
	projectile.id = id;
	projectile.owner = u.id;
	
	projectile.lifetime = 50;

	cooldown_left = cooldown;
}

void MachineGun::tick()
{
	if (cooldown_left > 0)
		cooldown_left -= 1;
}
