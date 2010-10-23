#include "medikit.h"

Location Medikit::bb_top()
{
	return Location(pos.x+1, pos.y+1, pos.z+1);
}

Location Medikit::bb_bot()
{
	return Location(pos.x-1, pos.y, pos.z-1);
}

void Medikit::collides(OctreeObject&) {
	std::cerr << "medikit collided!!" <<std::endl;
}
