#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>
#include <string>

#include "location.h"
#include "apomath.h"
#include "unit.h"

using namespace std;

class Projectile
{
	public:
		Projectile():
			lifetime(0)
		{
		}

		Location velocity;
		Location position;

		int lifetime;

		bool collides(const Unit& unit) const;
};

#endif

