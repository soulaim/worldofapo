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

class Projectile : public MovableObject, public HasProperties
{
    friend class BallisticWeaponUsage;
    friend class BeamWeaponUsage;
    friend class ProjectileTicker;

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

        const Location& getPosition() {
            return position;
        }

        const Location& getVelocity() {
            return velocity;
        }

		void tick();

		bool collides(const Unit&);
		bool collidesTerrain(Level& lvl) const;

		std::string copyOrder(int ID) const;
		void handleCopyOrder(std::stringstream& ss);
	private:
};

#endif

