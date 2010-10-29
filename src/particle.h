
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
		prev_pos = pos;
		pos += vel;
		vel.y -= FixedPoint(20, 1000);
		--cur_life;
	}
	
	float getAlpha()
	{
		return float(cur_life) / max_life;
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

public:
	ParticleSystem(int systemLife, int spawnRate_, int spawnVariance_, const Location& systemPos_, const Location& systemVel_,
				   const Location& particleVel_, int particleLife, float* startColor_, float* endColor_
				   ):
				   systemLifeTime(systemLife), particleLifeTime(particleLife), systemPos(systemPos_), systemVel(systemVel_), particleVel(particleVel_),
				   spawnRate(spawnRate_), spawnVariance(spawnVariance_)
	{
		startColor[0] = startColor_[0];
		startColor[1] = startColor_[1];
		startColor[2] = startColor_[2];
		startColor[3] = startColor_[3];
		
		endColor[0] = endColor_[0];
		endColor[1] = endColor_[1];
		endColor[2] = endColor_[2];
		endColor[3] = endColor_[3];
	}
	
	// graphics system needs to access this
	std::vector<Particle> particles;
	
	bool systemAlive()
	{
		return !(particles.empty() && (systemLifeTime < 1));
	}
	
	void makeParticle()
	{
		Particle p;
		p.max_life = particleLifeTime;
		p.cur_life = particleLifeTime;
		
		p.pos      = systemPos;
		p.prev_pos = systemPos;
		p.vel      = particleVel + systemVel;
		p.scale    = 0.1;
		
		p.r        = startColor[0];
		p.g        = startColor[1];
		p.b        = startColor[2];
		p.a        = startColor[3];
		
		particles.push_back(p);
	}
	
	void tick()
	{
		// update current particles
		for(size_t i=0; i<particles.size(); ++i)
		{
			particles[i].tick();
			if(!particles[i].alive())
			{
				particles[i] = particles.back();
				particles.pop_back();
				--i;
			}
		}
		
		// create new particles
		int particles = spawnRate;
		while(--particles)
		{
			makeParticle();
		}
		
		--systemLifeTime;
	}
	
private:
	int systemLifeTime;
	int particleLifeTime;
	
	Location systemPos;
	Location systemVel;
	
	Location particleVel;
	
	std::string textureName;
	
	int spawnRate, spawnVariance;
	
	float startColor[4];
	float endColor[4];
};


class ParticleSystemManager
{
public:
	std::vector<ParticleSystem> systems;
	
	void tick()
	{
		for(size_t i=0; i<systems.size(); i++)
			if(systems[i].systemAlive())
				systems[i].tick();
		if(!systems.empty())
			if(!systems.back().systemAlive())
				systems.pop_back();
	}
};



#endif

