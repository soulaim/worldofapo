
#ifndef H_WEAPON
#define H_WEAPON

#include <string>
#include <map>
#include <fstream>

#include "item.h"
#include "fixed_point.h"
#include "location.h"
#include "projectile.h"

class World;
class Unit;

class Weapon : public Item, public HasProperties
{
	World& world;
	Unit& unit;
	
	Projectile proto_projectile;
public:
	
	// ALERT: THIS IS HERE ONLY TO KEEP MEDIKIT OPERATIONAL..
	Weapon(World& w, Unit& u):
		world(w),
		unit(u)
	{
	}
	
	// THIS IS THE TRUE WAY TO GO
	Weapon(World& w, Unit& u, const std::string& fileName):
		world(w),
		unit(u)
	{
		load(fileName);
		
		if(intVals["CHILD_TPF"] == 0)
			intVals["CHILD_TPF"] = 1;
		
		generatePrototypeProjectile();
	}
	
	void generatePrototypeProjectile();
	void fire();
	
	void tick()
	{
		if(intVals["CD_LEFT"] > 0)
			--intVals["CD_LEFT"];
	}
	
	void onUse()
	{
		fire();
	}
};

#endif

