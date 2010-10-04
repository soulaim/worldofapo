#include "projectile.h"

bool Projectile::collides(const Unit& unit) const
{
	return position.near(unit.position, FixedPoint(3)/FixedPoint(1));
}

