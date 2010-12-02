
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
	Projectile proto_projectile;
public:
	
	Weapon(const std::string& fileName)
	{
		load(fileName);
		
		if(intVals["CHILD_TPF"] == 0)
			intVals["CHILD_TPF"] = 1;
		
		generatePrototypeProjectile();
	}
	
	void generatePrototypeProjectile();
	
	void prepareReload(Unit& user);
	void fire(World& world, Unit& user);
	
	void onUse(World& world, Unit& user);
	void tick();

	bool isReloading();
	bool onCooldown();
	
};

#endif

