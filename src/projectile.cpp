
#include "projectile.h"
#include "algorithms.h"

using namespace std;

bool Projectile::collides(const Unit& unit) const
{
	FixedPoint lengthVal = pointInLinesegment(curr_position, prev_position, unit.position);
	FixedPoint distance;
	if(lengthVal > FixedPoint(0) && lengthVal < FixedPoint(1))
		distance = (unit.position - curr_position).crossProduct(unit.position - prev_position).length() / (prev_position - curr_position).length();
	else
		distance = (curr_position - unit.position).length();
	
	return distance <= (FixedPoint(3)/FixedPoint(1));
}

bool Projectile::collidesTerrain(Level& lvl) const
{
	return curr_position.h <= lvl.getHeight(curr_position.x, curr_position.y);
}

void Projectile::tick()
{
	prev_position = curr_position;
	curr_position += velocity;
	--lifetime;
}

void Projectile::handleCopyOrder(stringstream& ss)
{
	ss >> curr_position.x.number >> curr_position.y.number >> curr_position.h.number >>
	velocity.x.number >> velocity.y.number >> velocity.h.number >>
	lifetime;
}

string Projectile::copyOrder(int ID)
{
	stringstream projectile_msg;
	projectile_msg << "-2 PROJECTILE " << ID << " " << curr_position.x.number << " " << curr_position.y.number << " " << curr_position.h.number << " " << velocity.x.number << " " << velocity.y.number << " " << velocity.h.number << " " << lifetime << "#";
	
	return projectile_msg.str();
}
