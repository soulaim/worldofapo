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
		
		std::string& operator() (const std::string& a)
		{
			return strVals[a];
		}
		
		Location velocity;
		Location curr_position;
		Location prev_position;
		
		bool destroyAfterFrame; // this does not need to be transmitted (if value changes => projectile is erased before the frame tick ends)
		
		// handy for transmitting stuff :D
		std::map<std::string, int> intVals;
		std::map<std::string, std::string> strVals;
		
		void tick();
		
		bool collides(const Unit&);
		bool collidesTerrain(Level& lvl) const;
		std::string copyOrder(int);
		void handleCopyOrder(std::stringstream& ss);
};

#endif

