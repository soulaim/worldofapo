#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>
#include <string>
#include <sstream>

#include "location.h"
#include "level.h"
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
		Location curr_position;
		Location prev_position;
		
		int owner;
		int lifetime;
		void tick();
		
		bool collides(const Unit& unit) const;
		bool collidesTerrain(Level& lvl) const;
		std::string copyOrder(int);
		void handleCopyOrder(std::stringstream& ss);
};

#endif

