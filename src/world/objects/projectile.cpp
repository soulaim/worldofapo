
#include "projectile.h"
#include "algorithms.h"
#include "unit.h"

using namespace std;

bool Projectile::collides(const Unit& unit)
{
	if(intVals["COLLISION_DETECTION_RADIUS"] == 0)
	{
		return Collision::lineBox(unit.bb_bot(), unit.bb_top(), position, prev_position);
	}
	else
	{
		return Collision::lineBoxRadius(unit.bb_bot(), unit.bb_top(), position, prev_position, FixedPoint(intVals["COLLISION_DETECTION_RADIUS"], 1000));
	}
}

bool Projectile::collidesTerrain(Level& lvl) const
{
	return position.y <= lvl.getHeight(position.x, position.z);
}

void Projectile::tick()
{
	prev_position = position;
	position += velocity;
	--intVals["LIFETIME"];
}

void Projectile::handleCopyOrder(stringstream& ss)
{
	ss >> position.x >> position.z >> position.y >> velocity.x >> velocity.z >> velocity.y;

	HasProperties::handleCopyOrder(ss);
}

string Projectile::copyOrder(int ID) const
{
	stringstream projectile_msg;
	projectile_msg << "-2 PROJECTILE " << ID << " " << prototype_model << " " << position.x << " " << position.z << " " << position.y << " " << velocity.x << " " << velocity.z << " " << velocity.y;

	projectile_msg << HasProperties::copyOrder();

	projectile_msg << "#";

	return projectile_msg.str();
}
