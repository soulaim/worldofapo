
#include "unit.h"
#include "world.h"
#include "weapon.h"

void Weapon::fire()
{
	if(intVals["CD_LEFT"] > 0)
	{
		return;
	}
	
	// TODO: Should be defined in the weapon definition, not constant
	u.soundInfo = "shoot";
	
	
	Location weapon_position = u.getPosition();
	Location projectile_direction = u.getLookDirection();
	
	weapon_position.y += 4;
	projectile_direction.y += 4;
	
	for(int i=0; i<intVals["PROJECTILES_PER_USE"]; ++i)
	{
		int id = w.nextUnitID();
		w.addProjectile(weapon_position, id);
		Projectile& projectile = w.projectiles[id];
		
		projectile["ID"]     = id;
		projectile["OWNER"]  = u.id;
		
		
		
		// set some properties first
		
		for(auto iter = intVals.begin(); iter != intVals.end(); iter++)
			if(iter->first.substr(0, 5) == "CHILD")
				projectile[iter->first.substr(6)] = iter->second;
		
		for(auto iter = strVals.begin(); iter != strVals.end(); iter++)
			if(iter->first.substr(0, 5) == "CHILD")
				projectile(iter->first.substr(6)) = iter->second;
		
		projectile["MAX_LIFETIME"] = projectile["LIFETIME"];
		projectile("NAME") = strVals["NAME"];
		
		// need to move projectile out of self-range (don't want to shoot self LOL)
		projectile_direction.normalize();
		projectile.velocity = projectile_direction * FixedPoint(9, 2); // this might also mean that it's impossible to hit a target that is very close, since the bullet spawns on the other side.
		projectile.tick();
		
		assert((projectile["TPF"] != 0) && strVals["NAME"].c_str());
		
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

