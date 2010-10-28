#ifndef H_MEDIWEAPON
#define H_MEDIWEAPON

#include "weapon.h"
#include "fixed_point.h"

class World;
class Unit;

class MedikitWeapon : public Weapon
{
public:
	MedikitWeapon(World& _w, Unit& _u):
		w(_w),
		u(_u),
		cooldown_left(0),
		cooldown(25)
	{
	}

	World& w;
	Unit& u;

	FixedPoint getFriction();

	int cooldown_left;
	int cooldown;

	void tick();
	void fire();

};

#endif

