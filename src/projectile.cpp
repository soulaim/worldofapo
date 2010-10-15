
#include "projectile.h"
#include "algorithms.h"

using namespace std;


bool Projectile::collides(const Unit& unit)
{
	return Collision::lineBox(unit.hitbox_bot(), unit.hitbox_top(), curr_position, prev_position);
}

bool Projectile::collidesTerrain(Level& lvl) const
{
	return curr_position.y <= lvl.getHeight(curr_position.x, curr_position.z);
}

void Projectile::tick()
{
	prev_position = curr_position;
	curr_position += velocity;
	--lifetime;
}

void Projectile::handleCopyOrder(stringstream& ss)
{
	ss >> curr_position.x >> curr_position.z >> curr_position.y >> velocity.x >> velocity.z >> velocity.y >> lifetime;
}

string Projectile::copyOrder(int ID)
{
	stringstream projectile_msg;
	projectile_msg << "-2 PROJECTILE " << ID << " " << curr_position.x << " " << curr_position.z << " " << curr_position.y << " " << velocity.x << " " << velocity.z << " " << velocity.y << " " << lifetime << "#";
	
	return projectile_msg.str();
}
