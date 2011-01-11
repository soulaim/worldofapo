
#include "unit.h"
#include "world.h"
#include "weapon.h"
#include "graphics/visualworld.h"


inline int min(const int a, const int b)
{
	return (a < b)?a:b;
}


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

void Weapon::onSecondaryActivate(World& world, Unit&)
{
	world.add_message("^Rweapons do not have secondary actions.");
}

// returns true if collecting succeeded, false if could not collect
bool Weapon::onCollect(World&, Unit& unit)
{
	for(size_t i=0; i<unit.weapons.size(); i++)
	{
		if(unit.weapons[i]("NAME") == strVals["NAME"])
		{
			unit.intVals[unit.weapons[i]("AMMUNITION_TYPE")] += 30;
			return true;
		}
	}
	
	unit.weapons.push_back(*this);
	return true;
}

void Weapon::onDrop(World& world, Unit&)
{
	// not really sure whether this is of any use
	world.add_message("^Rweapons can not be dropped");
}

void Weapon::onConsume(World& world, Unit&)
{
	world.add_message("^Rweapons can not be consumed");
}


void Weapon::onActivate(World& world, Unit& user)
{
	if(intVals["CLIP_BULLETS"] == 0)
	{
		prepareReload(user);
		return;
	}
	
	if(onCooldown() > FixedPoint(0) || isReloading() > FixedPoint(0))
		return;
	
	fire(world, user);
}

void Weapon::tick(Unit& unit)
{
	if(intVals["CD_LEFT"] > 0)
		--intVals["CD_LEFT"];
	
	if(intVals["RELOAD"])
	{
		if(--intVals["RELOAD"] == 0)
			reloadComplete(unit);
	}
}

FixedPoint Weapon::isReloading()
{
	return FixedPoint(intVals["RELOAD"], intVals["RELOAD_TIME"]);
}

FixedPoint Weapon::onCooldown()
{
	return FixedPoint(intVals["CD_LEFT"], intVals["COOLDOWN"]);
}

void Weapon::reloadComplete(Unit& user)
{
	std::string& ammotype = strVals["AMMUNITION_TYPE"];
	int& ammo             = user.intVals[ammotype];
	int& clip_ammo        = intVals["CLIP_BULLETS"];
	int ammo_change       = min(intVals["CLIP_SIZE"] - clip_ammo, ammo);
	
	clip_ammo += ammo_change;
	ammo      -= ammo_change; // reduce ammunition
}


void Weapon::prepareReload(Unit& user)
{
	if(isReloading() > FixedPoint(0)) // don't restart reloading process..
		return;
	
	std::string& ammotype = strVals["AMMUNITION_TYPE"];
	int& ammo             = user.intVals[ammotype];
	int& clip_ammo        = intVals["CLIP_BULLETS"];
	int ammo_change       = min(intVals["CLIP_SIZE"] - clip_ammo, ammo);
	
	// if theres nothing to reload, don't reload.
	if(ammo_change == 0)
		return;
	
	intVals["RELOAD"] = intVals["RELOAD_TIME"];
}

void Weapon::fire(World& world, Unit& user)
{
	// shooting may now begin!
	--intVals["CLIP_BULLETS"];
	
	user.soundInfo = strVals["FIRE_SOUND"];
	
	Location weapon_position = user.getPosition();
	Location projectile_direction = user.getLookDirection();
	
	weapon_position.y += FixedPoint(4) + FixedPoint(intVals["Y_OFFSET"], 1000);
	user.velocity -= projectile_direction * FixedPoint(intVals["FIRE_KICK"], 1000) / FixedPoint(user.intVals["MASS"], 1000);
	
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

