
#ifndef _H_PARTICLE_STUFF
#define _H_PARTICLE_STUFF

#include "location.h"
#include "frustum/Vec3.h"

class Particle
{
	public:
	
	int max_life;
	int cur_life;
	Location pos;
	Location vel;
	
	void tick()
	{
		pos += vel;
		vel.y.number -= 20;
	}
	
	float getAlpha()
	{
		return float(cur_life) / max_life;
	}
	
	void decrementLife()
	{
		--cur_life;
	}
	
	bool alive()
	{
		return cur_life > 0;
	}
	
	void updateDepthVal(Vec3& d)
	{
		depthVal = d.x * pos.x.getFloat() + d.y * pos.y.getFloat() + d.z * pos.z.getFloat();
	}
	
	bool operator < (const Particle& a) const
	{
		return depthVal > a.depthVal;
	}
	
	float depthVal;
	float r;
	float g;
	float b;
	float a;
};

#endif

