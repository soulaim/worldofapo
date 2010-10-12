
#include "projectile.h"
#include "algorithms.h"

using namespace std;

// from http://www.3dkingdoms.com/weekly/weekly.php?a=3
int Projectile::GetIntersection(FixedPoint fDst1, FixedPoint fDst2, Location P1, Location P2, Location &Hit) {
	if ( (fDst1 * fDst2) >= 0.0f) return 0;
	if ( fDst1 == fDst2) return 0;
	Hit = P1 + (P2-P1) * ( -fDst1/(fDst2-fDst1) );
	return 1;
}

int Projectile::InBox(Location Hit, Location B1, Location B2, const int Axis)
{
	if ( Axis==1 && Hit.z > B1.z && Hit.z < B2.z && Hit.y > B1.y && Hit.y < B2.y) return 1;
	if ( Axis==2 && Hit.z > B1.z && Hit.z < B2.z && Hit.x > B1.x && Hit.x < B2.x) return 1;
	if ( Axis==3 && Hit.x > B1.x && Hit.x < B2.x && Hit.y > B1.y && Hit.y < B2.y) return 1;
	return 0;
}

// returns true if line (L1, L2) intersects with the box (B1, B2)
int Projectile::CheckLineBox(Location B1, Location B2, Location L1, Location L2)
{
	if (L2.x < B1.x && L1.x < B1.x) return false;
	if (L2.x > B2.x && L1.x > B2.x) return false;
	if (L2.y < B1.y && L1.y < B1.y) return false;
	if (L2.y > B2.y && L1.y > B2.y) return false;
	if (L2.z < B1.z && L1.z < B1.z) return false;
	if (L2.z > B2.z && L1.z > B2.z) return false;

	if (L1.x > B1.x && L1.x < B2.x && L1.y > B1.y && L1.y < B2.y &&
	    L1.z > B1.z && L1.z < B2.z)
		return true;

	Location Hit;
	if ( (GetIntersection( L1.x-B1.x, L2.x-B1.x, L1, L2, Hit) && InBox( Hit, B1, B2, 1 ))
	  || (GetIntersection( L1.y-B1.y, L2.y-B1.y, L1, L2, Hit) && InBox( Hit, B1, B2, 2 ))
	  || (GetIntersection( L1.z-B1.z, L2.z-B1.z, L1, L2, Hit) && InBox( Hit, B1, B2, 3 ))
	  || (GetIntersection( L1.x-B2.x, L2.x-B2.x, L1, L2, Hit) && InBox( Hit, B1, B2, 1 ))
	  || (GetIntersection( L1.y-B2.y, L2.y-B2.y, L1, L2, Hit) && InBox( Hit, B1, B2, 2 ))
	  || (GetIntersection( L1.z-B2.z, L2.z-B2.z, L1, L2, Hit) && InBox( Hit, B1, B2, 3 )))
		return true;
	return false;
}

bool Projectile::collides(const Unit& unit)
{
	return CheckLineBox(unit.hitbox_bot(), unit.hitbox_top(), curr_position, prev_position);
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
