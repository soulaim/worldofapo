
#ifndef _H_PARTICLE_STUFF
#define _H_PARTICLE_STUFF

#include "location.h"
#include "frustum/vec3.h"

class Particle
{
public:
	
	// could be used for game state
	int max_life;
	int cur_life;
	
	// could be used for game state
	Location pos, target_pos;
	Location vel;
	
	// graphics only
	float depthVal;
	float scale;
	float sr, er;
	float sg, eg;
	float sb, eb;
	float a;
	
	void viewTick()
	{
		pos += (target_pos - pos) * FixedPoint(1, 5);
	}
	
	void tick()
	{
		target_pos += vel;
		vel.y -= FixedPoint(20, 1000);
		--cur_life;
	}

	float getScale() const
	{
		// Don't scale too big.
		return scale * 1.5f;
	}
	
	float getAlpha() const
	{
		return float(cur_life) / (max_life + 1);
	}
	
	void getColor(float* colors) const
	{
		float relativeLife = float(cur_life) / (max_life + 1);
		colors[0] = er + (sr - er) * relativeLife;
		colors[1] = eg + (sg - eg) * relativeLife;
		colors[2] = eb + (sb - eb) * relativeLife;
	}
	
	void getColor(float& r, float& g, float& b) const
	{
		float relativeLife = float(cur_life) / (max_life + 1);
		r = er + (sr - er) * relativeLife;
		g = eg + (sg - eg) * relativeLife;
		b = eb + (sb - eb) * relativeLife;
	}
	
	bool alive() const
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

#endif

