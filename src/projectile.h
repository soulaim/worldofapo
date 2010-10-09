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
		
		bool collides(const Unit& unit);
		bool collidesTerrain(Level& lvl) const;
		std::string copyOrder(int);
		void handleCopyOrder(std::stringstream& ss);
		int GetIntersection(FixedPoint fDst1, FixedPoint fDst2, Location P1, Location P2, Location &Hit);
		int InBox(Location Hit, Location B1, Location B2, const int Axis);
		int CheckLineBox(Location B1, Location B2, Location L1, Location L2);
};

#endif

