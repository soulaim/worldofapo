
#ifndef _H_PARTICLE_STUFF
#define _H_PARTICLE_STUFF

#include "location.h"
#include "frustum/Vec3.h"

class Particle
{
public:
	
	// could be used for game state
	int max_life;
	int cur_life;
	
	// could be used for game state
	Location pos, prev_pos;
	Location vel;
	
	// graphics only
	float depthVal;
	float scale;
	float r;
	float g;
	float b;
	float a;
	
	void tick()
	{
		pos += vel;
		vel.y -= FixedPoint(20, 1000);
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
};


// describes 
class ParticleSystem
{
	std::string texture_name;
	std::vector<Particle> particles;
	
	float startColor[4];
	float endColor[4];
	
	int lifeTime;
};

#endif

