
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
	World& w;
	Unit& u;
	
	Projectile proto_projectile;
public:
	
	// THIS IS THE TRUE WAY TO GO
	Weapon(World& _w, Unit& _u, const std::string& fileName):
	w(_w),
	u(_u)
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

