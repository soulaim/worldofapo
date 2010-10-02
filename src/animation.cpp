
#include "animation.h"
#include <iostream>

using namespace std;

void Animation::getAnimationState(int aniIndex, float& xrot, float& yrot, float& zrot)
{
	unsigned size = rot_x.size();
	
	if(size == 0)
	{
		// cerr << "ERROR: animation of size 0 was requested for data!" << endl;
		return;
	}
	
	xrot = rot_x[ aniIndex % size ];
	yrot = rot_y[ aniIndex % size ];
	zrot = rot_z[ aniIndex % size ];
	return;
}

void Animation::setAnimationState(int aniIndex, float xrot, float yrot, float zrot)
{
	unsigned size = rot_x.size();
	
	if(size <= aniIndex)
	{
		rot_x.resize(aniIndex + 1, 0.f);
		rot_y.resize(aniIndex + 1, 0.f);
		rot_z.resize(aniIndex + 1, 0.f);
		return;
	}
	
	rot_x[ aniIndex ] = xrot;
	rot_y[ aniIndex ] = yrot;
	rot_z[ aniIndex ] = zrot;
	return;
}

void Animation::insertAnimationState(float x, float y, float z)
{
	rot_x.push_back(x);
	rot_y.push_back(y);
	rot_z.push_back(z);
	return;
}

int Animation::getSize()
{
	return rot_x.size();
}

