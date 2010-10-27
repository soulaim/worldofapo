#ifndef H_MEDIWEAPON
#define H_MEDIWEAPON

#include "weapon.h"

class World;
class Unit;

class MedikitWeapon : public Weapon
{
public:
	MedikitWeapon(World& _w, Unit& _u):
		w(_w),
		u(_u),
		cooldown_left(0),
		cooldown(2)
	{
	}

	World& w;
	Unit& u;

	int cooldown_left;
	int cooldown;

	void tick();
	void fire();

};

#endif

