#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "physics/collision.h"
#include "misc/vec3.h"
#include "world/level.h"
#include "misc/apomath.h"
#include "physics/movable_object.h"
#include "misc/hasproperties.h"

#include <vector>
#include <string>
#include <sstream>
#include <map>

class Unit;

class Projectile : MovableObject, HasProperties
{
	friend class Weapon;
	friend class World;
	friend class GameView;
    friend class BallisticWeaponUsage;
    friend class BeamWeaponUsage;

	public:
		Projectile():
			destroyAfterFrame(false),
			prototype_model(0)
		{
		}

		bool destroyAfterFrame; // this does not need to be transmitted (if value changes => projectile is erased before the frame tick ends)
		size_t prototype_model;

		const Location& getPosition() const
		{
			return position;
		}

		void tick();

		bool collides(const Unit&);
		bool collidesTerrain(Level& lvl) const;

		std::string copyOrder(int ID) const;
		void handleCopyOrder(std::stringstream& ss);
	private:
};

#endif

