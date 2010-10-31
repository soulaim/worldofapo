
#include "projectile.h"
#include "algorithms.h"
#include "unit.h"

using namespace std;

bool Projectile::collides(const Unit& unit)
{
	return Collision::lineBox(unit.bb_bot(), unit.bb_top(), curr_position, prev_position);
}

bool Projectile::collidesTerrain(Level& lvl) const
{
	return curr_position.y <= lvl.getHeight(curr_position.x, curr_position.z);
}

void Projectile::tick()
{
	prev_position = curr_position;
	curr_position += velocity;
	--intVals["LIFETIME"];
}

void Projectile::handleCopyOrder(stringstream& ss)
{
	ss >> curr_position.x >> curr_position.z >> curr_position.y >> velocity.x >> velocity.z >> velocity.y;
	
	string key;
	while(ss >> key)
	{
		if(key == "STR_VALS")
			break;
		ss >> intVals[key];
	}
	
	while(ss >> key)
	{
		ss >> strVals[key];
	}
	
}

string Projectile::copyOrder(int ID)
{
	stringstream projectile_msg;
	projectile_msg << "-2 PROJECTILE " << ID << " " << curr_position.x << " " << curr_position.z << " " << curr_position.y << " " << velocity.x << " " << velocity.z << " " << velocity.y;
	
	for(auto iter = intVals.begin(); iter != intVals.end(); iter++)
		projectile_msg << " " << iter->first << " " << iter->second;
	projectile_msg << " STR_VALS";
	for(auto iter = strVals.begin(); iter != strVals.end(); iter++)
		projectile_msg << " " << iter->first << " " << iter->second;
	projectile_msg << "#";
	
	return projectile_msg.str();
}
