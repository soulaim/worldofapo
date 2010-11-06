
#include "particle_source.h"

using namespace std;

string& ParticleSource::getStrProperty(const string& a)
{
	return (*this)(a);
}

int& ParticleSource::getIntProperty(const string& a)
{
	return (*this)[a];
}

const string& ParticleSource::getStrProperty(const string& a) const
{
	return (*this)(a);
}

const int& ParticleSource::getIntProperty(const string& a) const
{
	return (*this)[a];
}



void ParticleSource::tick(std::vector<Particle>& particles)
{
	float relativeLife = float(getIntProperty("CUR_LIFE")) / getIntProperty("MAX_LIFE");
	
	float start_red   = getIntProperty("SRED") / 256.f;
	float start_green = getIntProperty("SGREEN") / 256.f;
	float start_blue  = getIntProperty("SBLUE") / 256.f;
	
	float end_red   = getIntProperty("ERED") / 256.f;
	float end_green = getIntProperty("EGREEN") / 256.f;
	float end_blue  = getIntProperty("EBLUE") / 256.f;
	
	float now_r = end_red   + relativeLife * (start_red   - end_red);
	float now_g = end_green + relativeLife * (start_green - end_green);
	float now_b = end_blue  + relativeLife * (start_blue  - end_blue);
	
	int particlesPerFrame = getIntProperty("PPF");
	for(int i=0; i<particlesPerFrame; ++i)
	{
		Particle p;
		p.r = now_r;
		p.g = now_g;
		p.b = now_b;
		p.a = relativeLife;
		
		p.scale = 0.5 * relativeLife;
		p.max_life = getIntProperty("PLIFE");
		p.cur_life = p.max_life;
		
		p.pos = position;
		p.vel = velocity;
		
		float max_rand = 0.02f * getIntProperty("MAX_RAND");
		p.vel.x += FixedPoint(getIntProperty("VEL_X_TOP"), getIntProperty("VEL_X_BOT")) * FixedPoint(((rand() % 1000) - 500) * max_rand, 1000);
		p.vel.y += FixedPoint(getIntProperty("VEL_Y_TOP"), getIntProperty("VEL_Y_BOT")) * FixedPoint(((rand() % 1000) - 500) * max_rand, 1000);
		p.vel.z += FixedPoint(getIntProperty("VEL_Z_TOP"), getIntProperty("VEL_Z_BOT")) * FixedPoint(((rand() % 1000) - 500) * max_rand, 1000);
		
		particles.push_back(p);
	}
	
	--getIntProperty("CUR_LIFE");
}



bool ParticleSource::alive() const
{
	return getIntProperty("CUR_LIFE") > 0;
}

