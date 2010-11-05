
#include "unit.h"
#include "world.h"
#include "weapon.h"

void Weapon::fire()
{
	if(intVals["CD_LEFT"] > 0)
	{
		return;
	}
	
	// TODO: this isn't actually a property for the unit :(
	u.soundInfo = "shoot";
	
	
	int angle   = u.angle;
	int upangle = u.upangle;
	
	FixedPoint cos = w.apomath.getCos(angle);
	FixedPoint sin = w.apomath.getSin(angle);
	FixedPoint upcos = w.apomath.getCos(upangle);
	FixedPoint upsin = w.apomath.getSin(upangle);
	
	Location relative_position;
	FixedPoint x = 30;
	FixedPoint y = 0;
	FixedPoint z = 0;
	
	relative_position.x = cos * upcos * x - sin * z + cos * upsin * y;
	relative_position.z = sin * upcos * x + cos * z + sin * upsin * y;
	relative_position.y =      -upsin * x           +       upcos * y;
	
	Location weapon_position = u.getPosition();
	Location projectile_direction = relative_position;

	weapon_position.y += 4;
	projectile_direction.y += 4;
	
	for(int i=0; i<intVals["PROJECTILES_PER_USE"]; ++i)
	{
		int id = w.nextUnitID();
		w.addProjectile(weapon_position, id);
		Projectile& projectile = w.projectiles[id];
		
		// set some properties first
		projectile["EXPLODE_POWER"] = intVals["CHILD_EXPLODE_POWER"];
		projectile("AT_DEATH") = strVals["CHILD_AT_DEATH"];
		
		projectile["MASS"] = intVals["CHILD_MASS"];
		projectile["TPF"]    = intVals["CHILD_TPF"];
		projectile["DAMAGE"] = intVals["CHILD_DAMAGE"];
		projectile["ID"]     = id;
		projectile["OWNER"]  = u.id;
		projectile["LIFETIME"] = intVals["CHILD_LIFETIME"];
		
		// need to move projectile out of self-range (don't want to shoot self LOL)
		projectile_direction.normalize();
		projectile.velocity = projectile_direction * FixedPoint(7, 2);
		projectile.tick();
		
		FixedPoint speedPerTick(intVals["CHILD_SPEED_TOP"], intVals["CHILD_SPEED_BOT"]);
		projectile.velocity = projectile_direction * speedPerTick + u.getVelocity() / projectile["TPF"];
		
		// variance term for velocity
		if(intVals["HAS_VARIANCE"])
		{
			FixedPoint max_var(intVals["CHILD_SPEED_VAR_TOP"], intVals["CHILD_SPEED_VAR_BOT"]);
			FixedPoint half_var = max_var / FixedPoint(2);
			
			FixedPoint rnd_x = FixedPoint( (id * (1 | 16 | 64)) & 127, 127);
			FixedPoint rnd_y = FixedPoint( (id * (2 | 8 | 16)) & 127, 127);
			FixedPoint rnd_z = FixedPoint( (id * (1 | 4 | 32)) & 127, 127);
			
			// add variance term
			projectile.velocity.x += rnd_x * max_var - half_var;
			projectile.velocity.y += rnd_y * max_var - half_var;
			projectile.velocity.z += rnd_z * max_var - half_var;
		}
	}
	
	intVals["CD_LEFT"] = intVals["COOLDOWN"];
}

