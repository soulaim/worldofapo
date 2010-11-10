
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
	static unsigned semiUniqueNumber = 1;
	
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
		p.a = 1.0f;
		
		p.scale = 0.5;
		p.max_life = getIntProperty("PLIFE");
		p.cur_life = p.max_life;
		
		p.pos = position;
		p.target_pos = position;
		p.vel = velocity;
		
		FixedPoint max_var(intVals["PSP_1000"], 1000);
		max_var *= FixedPoint(getIntProperty("CUR_LIFE"), getIntProperty("MAX_LIFE"));
		FixedPoint half_var = max_var / FixedPoint(2);
		
		FixedPoint rnd_x = FixedPoint( (semiUniqueNumber * (1 | 16 | 64)) & 127, 127);
		FixedPoint rnd_y = FixedPoint( (semiUniqueNumber * (2 | 8 | 16))  & 127, 127);
		FixedPoint rnd_z = FixedPoint( (semiUniqueNumber * (1 | 4 | 32))  & 127, 127);
		
		// add variance term
		p.vel.x += rnd_x * max_var - half_var;
		p.vel.y += rnd_y * max_var - half_var;
		p.vel.z += rnd_z * max_var - half_var;
		
		particles.push_back(p);
		++semiUniqueNumber;
	}
	
	--getIntProperty("CUR_LIFE");
}



bool ParticleSource::alive() const
{
	return getIntProperty("CUR_LIFE") > 0;
}

