#include "animation.h"

#include <iostream>
#include <cmath>
#include <map>
#include <fstream>
#include <iomanip>
#include <sstream>

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
		
		/*
		xrot = 0;
		yrot = 0;
		zrot = 0;
		*/
		
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


std::map<std::string, std::map<std::string, Animation> > ANIMATIONS;


Animation& Animation::getAnimation(const std::string& modelnode_name, const std::string& animation_name)
{
	// TODO: this can be made constant time with simple indexing, if necessary.
	return ANIMATIONS[modelnode_name][animation_name];
}


bool Animation::load(const std::string& filename)
{
	std::ifstream in(filename);
	if(!in)
	{
		return false;
	}
	string line;
	int line_count = 0;
	while(getline(in, line))
	{
		++line_count;
		if(line.empty())
		{
			continue;
		}

		string modelnode_name;
		string animation_name;
		size_t time;
		float rot_x;
		float rot_y;
		float rot_z;

		stringstream ss(line);
		ss >> modelnode_name >> animation_name >> time >> rot_x >> rot_y >> rot_z;
		if(!ss)
		{
			cerr << "BAD ANIMATION FILE FORMAT: '" << filename << "': " << line_count << "\n";
			return false;
		}
		Animation& animation = getAnimation(modelnode_name, animation_name);
		animation.insertAnimationState(time, rot_x, rot_y, rot_z);
	}
	return true;
}

bool Animation::save(const std::string& filename)
{
	std::ofstream out(filename);
	if(!out)
	{
		return false;
	}
	for(auto it = ANIMATIONS.begin(); it != ANIMATIONS.end(); ++it)
	{
		for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			Animation& animation = it2->second;
			for(size_t i = 0; i < animation.times.size(); ++i)
			{
				out << it->first << " " << it2->first << " ";
				out << fixed << setprecision(3);
				out << setw(4) << animation.times[i] << " " << setw(8) << animation.rot_x[i] << " " << setw(8) << animation.rot_y[i] << " " << setw(8) << animation.rot_z[i];
				out << "\n";
			}
		}
		out << "\n";
	}
	out.close();

	return true;
}

