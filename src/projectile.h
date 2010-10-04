#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>
#include <string>
#include <sstream>

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
		std::string copyOrder(int);
		void handleCopyOrder(std::stringstream& ss);
};

#endif

