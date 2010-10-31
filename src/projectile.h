#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <vector>
#include <string>
#include <sstream>
#include <map>

#include "collision.h"
#include "location.h"
#include "level.h"
#include "apomath.h"

class Unit;

class Projectile
{
	public:
		Projectile():
			destroyAfterFrame(false)
		{
			intVals["LIFETIME"] = 0;
		}
		
		int& operator [] (const std::string& a)
		{
			return intVals[a];
		}
		
		Location velocity;
		Location curr_position;
		Location prev_position;
		
		bool destroyAfterFrame; // this does not need to be transmitted (if value changes => projectile is erased before the frame tick ends)
		
		std::map<std::string, int> intVals;
		
		void tick();
		
		bool collides(const Unit&);
		bool collidesTerrain(Level& lvl) const;
		std::string copyOrder(int);
		void handleCopyOrder(std::stringstream& ss);
};

#endif

