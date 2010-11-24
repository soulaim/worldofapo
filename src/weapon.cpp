
#include "unit.h"
#include "world.h"
#include "visualworld.h"
#include "weapon.h"


void Weapon::generatePrototypeProjectile()
{
	// set some properties first
	for(auto iter = intVals.begin(); iter != intVals.end(); iter++)
		if(iter->first.substr(0, 5) == "CHILD")
			proto_projectile[iter->first.substr(6)] = iter->second;
	
	for(auto iter = strVals.begin(); iter != strVals.end(); iter++)
		if(iter->first.substr(0, 5) == "CHILD")
			proto_projectile(iter->first.substr(6)) = iter->second;
	
	proto_projectile["MAX_LIFETIME"] = proto_projectile["LIFETIME"];
	proto_projectile("NAME") = strVals["NAME"];
	
	proto_projectile["ID"] = 0;
	proto_projectile["OWNER"] = 0;

	proto_projectile.prototype_model = proto_projectile["MODEL_PROTOTYPE"];
}

void Weapon::fire()
{
	if(intVals["CD_LEFT"] > 0)
	{
		return;
	}
	
	unit->soundInfo = strVals["FIRE_SOUND"];
	
	Location weapon_position = unit->getPosition();
	Location projectile_direction = unit->getLookDirection();
	
	weapon_position.y += 4;
	// projectile_direction.y += 4;

	size_t model_prototype = proto_projectile.prototype_model;
	
	if(intVals["FIRE_LIGHT_LIFE"] > 0)
		world->visualworld->weaponFireLight(world->nextUnitID(), unit->position, intVals["FIRE_LIGHT_LIFE"], intVals["FIRE_LIGHT_R"], intVals["FIRE_LIGHT_G"], intVals["FIRE_LIGHT_B"]);
	
	for(int i=0; i<intVals["PROJECTILES_PER_USE"]; ++i)
	{
		int id = world->nextUnitID();
		world->addProjectile(weapon_position, id, model_prototype);
		Projectile& projectile = world->projectiles[id];
		
		projectile.intVals = proto_projectile.intVals;
		projectile.strVals = proto_projectile.strVals;
		
		projectile["ID"]     = id;
		projectile["OWNER"]  = unit->id;
		
		// need to move projectile out of self-range (don't want to shoot self LOL)
		projectile_direction.normalize();
		projectile.velocity = projectile_direction * FixedPoint(9, 2); // this might also mean that it's impossible to hit a target that is very close, since the bullet spawns on the other side.
		projectile.tick();
		
		assert((projectile["TPF"] != 0) && strVals["NAME"].c_str());
		
		FixedPoint speedPerTick(intVals["CHILD_SPEED_TOP"], intVals["CHILD_SPEED_BOT"]);
		projectile.velocity = projectile_direction * speedPerTick + unit->getVelocity() / projectile["TPF"];
		
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

