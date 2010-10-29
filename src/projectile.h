#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>
#include <string>
#include <sstream>

#include "collision.h"
#include "location.h"
#include "level.h"
#include "apomath.h"

class Unit;

class Projectile
{
	public:
		Projectile():
			destroyAfterFrame(false), lifetime(0)
		{
		}

		Location velocity;
		Location curr_position;
		Location prev_position;
		
		bool destroyAfterFrame; // this does not need to be transmitted (if value changes => projectile is erased before the frame tick ends)
		
		int id;
		int owner;
		int lifetime;
		
		void tick();
		
		bool collides(const Unit&);
		bool collidesTerrain(Level& lvl) const;
		std::string copyOrder(int);
		void handleCopyOrder(std::stringstream& ss);
};

#endif

