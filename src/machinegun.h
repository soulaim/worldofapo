#ifndef H_MACHINEGUN
#define H_MACHINEGUN

#include "fixed_point.h"
#include "location.h"
#include "projectile.h"
#include "weapon.h"

class World;
class Unit;

class MachineGun : public Weapon
{
public:
	MachineGun(World& _w, Unit& _u):
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

	void shoot();
	void tick();
};

#endif

