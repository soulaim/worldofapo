
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

void Weapon::onUse(World& world, Unit& user)
{
	fire(world, user);
}

void Weapon::tick()
{
	if(intVals["CD_LEFT"] > 0)
		--intVals["CD_LEFT"];
	
	if(intVals["RELOAD"])
		--intVals["RELOAD"];
}

inline int min(const int& a, const int& b)
{
	return (a < b)?a:b;
}

void Weapon::fire(World& world, Unit& user)
{
	std::string& ammotype = strVals["AMMUNITION_TYPE"];
	int& ammo             = user.intVals[ammotype];
	if(intVals["CD_LEFT"] > 0 || intVals["RELOAD"])
	{
		return;
	}
	
	if(intVals["CLIP_BULLETS"] == 0)
	{
		// if theres nothing to reload with, don't reload.
		if(ammo == 0)
			return;
		
		intVals["RELOAD"] = intVals["RELOAD_TIME"];
		int ammo_change = min(intVals["CLIP_SIZE"], ammo);
		intVals["CLIP_BULLETS"] = ammo_change;
		ammo -= ammo_change; // reduce ammunition
		return;
	}
	
	// shooting may now begin!
	--intVals["CLIP_BULLETS"];
	
	user.soundInfo = strVals["FIRE_SOUND"];
	
	Location weapon_position = user.getPosition();
	Location projectile_direction = user.getLookDirection();
	
	weapon_position.y += 4;
	// projectile_direction.y += 4;

	size_t model_prototype = proto_projectile.prototype_model;
	
	if(intVals["FIRE_LIGHT_LIFE"] > 0)
		world.visualworld->weaponFireLight(world.nextUnitID(), user.position, intVals["FIRE_LIGHT_LIFE"], intVals["FIRE_LIGHT_R"], intVals["FIRE_LIGHT_G"], intVals["FIRE_LIGHT_B"]);
	
	for(int i=0; i<intVals["PROJECTILES_PER_USE"]; ++i)
	{
		int id = world.nextUnitID();
		world.addProjectile(weapon_position, id, model_prototype);
		Projectile& projectile = world.projectiles[id];
		
		projectile.intVals = proto_projectile.intVals;
		projectile.strVals = proto_projectile.strVals;
		
		projectile["ID"]     = id;
		projectile["OWNER"]  = user.id;
		
		// need to move projectile out of self-range (don't want to shoot self LOL)
		projectile_direction.normalize();
		projectile.velocity = projectile_direction * FixedPoint(9, 2);
		projectile.tick();
		
		assert((projectile["TPF"] != 0) && strVals["NAME"].c_str());
		
		FixedPoint speedPerTick(intVals["CHILD_SPEED_TOP"], intVals["CHILD_SPEED_BOT"]);
		projectile.velocity = projectile_direction * speedPerTick + user.getVelocity() / projectile["TPF"];
		
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

