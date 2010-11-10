
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
	
	
	Location weapon_position = u.getPosition();
	Location projectile_direction = u.getLookDirection();
	
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
		projectile["MAX_LIFETIME"] = intVals["CHILD_LIFETIME"];
		
		projectile["AIR_RESISTANCE"] = intVals["CHILD_AIR_RESISTANCE"];
		projectile["GRAVITY"] = intVals["CHILD_GRAVITY"];
		
		projectile["PARTICLES_PER_FRAME"] = intVals["PARTICLES_PER_FRAME"];
		projectile["PARTICLE_VELOCITY"] = intVals["PARTICLE_VELOCITY"]; // multiple of the projectiles velocity per 1000
		projectile["PARTICLE_RAND_1000"] = intVals["PARTICLE_RAND_1000"]; // randomness per 1000
		projectile["PARTICLE_LIFE"] = intVals["PARTICLE_LIFE"];
		
		// define particle colors
		projectile("START_COLOR_START") = strVals["START_COLOR_START"];
		projectile("END_COLOR_START") = strVals["END_COLOR_START"];
		projectile("START_COLOR_END") = strVals["START_COLOR_END"];
		projectile("END_COLOR_END") = strVals["END_COLOR_END"];
		
		// need to move projectile out of self-range (don't want to shoot self LOL)
		projectile_direction.normalize();
		projectile.velocity = projectile_direction * FixedPoint(9, 2); // this might also mean that it's impossible to hit a target that is very close, since the bullet spawns on the other side.
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

