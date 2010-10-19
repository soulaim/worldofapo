#include "animation.h"

#include <iostream>
#include <cmath>

using namespace std;

Animation::Animation():
	total_time(0)
{
}

void Animation::getAnimationState(size_t time, float& xrot, float& yrot, float& zrot) const
{
	if(times.empty())
	{
//		cerr << "ERROR: animation of size 0 was requested for data!" << endl;
		return;
	}

	time %= total_time;
	const size_t slots = times.size();

	size_t current = 0;
	size_t left = time;
	while(left > times[current])
	{
		left -= times[current];
		current = (current + 1) % slots;
	}
//	cerr << "getanimstate, time: " << time << ", current: " << current << "\n";
	
	float new_xrot = rot_x[ current ];
	float new_yrot = rot_y[ current ];
	float new_zrot = rot_z[ current ];

	float new_xrot2 = new_xrot + 360.0f;
	float new_yrot2 = new_yrot + 360.0f;
	float new_zrot2 = new_zrot + 360.0f;

	float prev_xrot = rot_x[ (current + slots-1) % slots ];
	float prev_yrot = rot_y[ (current + slots-1) % slots ];
	float prev_zrot = rot_z[ (current + slots-1) % slots ];

	if(abs(new_xrot2 - prev_xrot) < abs(new_xrot - prev_xrot))
	{
		new_xrot = new_xrot2;
	}
	if(abs(new_yrot2 - prev_yrot) < abs(new_yrot - prev_yrot))
	{
		new_yrot = new_yrot2;
	}
	if(abs(new_zrot2 - prev_zrot) < abs(new_zrot - prev_zrot))
	{
		new_zrot = new_zrot2;
	}
	float t = float(left + 1) / times[current];

	xrot = new_xrot * t + (1-t) * prev_xrot;
	yrot = new_yrot * t + (1-t) * prev_yrot;
	zrot = new_zrot * t + (1-t) * prev_zrot;

	
}

/*
void Animation::setAnimationState(size_t aniIndex, float xrot, float yrot, float zrot)
{
	size_t size = rot_x.size();
	
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
}
*/
void Animation::insertAnimationState(size_t time, float x, float y, float z)
{
	rot_x.push_back(x);
	rot_y.push_back(y);
	rot_z.push_back(z);
	times.push_back(time);
	total_time += time;
}

size_t Animation::getSize() const
{
	return times.size();
}

size_t Animation::totalTime() const
{
	return total_time;
}

