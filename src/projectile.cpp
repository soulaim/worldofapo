#include "projectile.h"

using namespace std;

bool Projectile::collides(const Unit& unit) const
{
	return position.near(unit.position, FixedPoint(3)/FixedPoint(1));
}

bool Projectile::collidesTerrain(Level& lvl) const
{
	return position.h <= lvl.getHeight(position.x, position.y);
}

void Projectile::handleCopyOrder(stringstream& ss)
{
	ss >> position.x.number >> position.y.number >> position.h.number >>
	velocity.x.number >> velocity.y.number >> velocity.h.number >>
	lifetime;
}

string Projectile::copyOrder(int ID)
{
	stringstream projectile_msg;
	projectile_msg << "-2 PROJECTILE " << ID << " " << position.x.number << " " << position.y.number << " " << position.h.number << " " << velocity.x.number << " " << velocity.y.number << " " << velocity.h.number << " " << lifetime << "#";
	
	return projectile_msg.str();
}
