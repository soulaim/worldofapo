
#ifndef _H_PARTICLE_STUFF
#define _H_PARTICLE_STUFF

#include "misc/vec3.h"

class Particle
{
public:
	
	// could be used for game state
	int max_life;
	int cur_life;
	
	// graphics only
	vec3<float> pos, target_pos;
	vec3<float> vel;
	
	float depthVal;
	float scale;
	float sr, er;
	float sg, eg;
	float sb, eb;
	float a;
	
	void viewTick()
	{
		pos += (target_pos - pos) * 0.25f;
	}
	
	void tick()
	{
		target_pos += vel;
		vel.y -= 0.02f;
		--cur_life;
	}

	float getScale() const
	{
		// Don't scale too big.
		return scale * 1.5f * getAlpha();
	}
	
	float getAlpha() const
	{
		// float x     = float(cur_life) / (max_life + 1);
		float x_inv = float(max_life - cur_life) / (max_life + 1);
		return 1.f - x_inv * x_inv;
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
	
	void updateDepthVal(vec3<float>& d)
	{
		depthVal = d.x * pos.x + d.y * pos.y + d.z * pos.z;
	}
	
	bool operator < (const Particle& a) const
	{
		return depthVal > a.depthVal;
	}
};

#endif

