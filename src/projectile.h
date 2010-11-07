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
#include "movable_object.h"
#include "item.h"

class Unit;

class Projectile : MovableObject, HasProperties
{
	friend class Weapon;
	friend class World;
	friend class Graphics; // only because of draw debug projectiles
	
	public:
		Projectile():
			destroyAfterFrame(false)
		{
		}
		
		bool destroyAfterFrame; // this does not need to be transmitted (if value changes => projectile is erased before the frame tick ends)

		
		const Location& getPosition()
		{
			return position;
		}
		
		void tick();
		
		bool collides(const Unit&);
		bool collidesTerrain(Level& lvl) const;
		std::string copyOrder(int);
		void handleCopyOrder(std::stringstream& ss);
};

#endif

