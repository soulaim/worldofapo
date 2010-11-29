
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
	
	float start_red   = intVals["SRED"] / 256.f;
	float start_green = intVals["SGREEN"] / 256.f;
	float start_blue  = intVals["SBLUE"] / 256.f;
	
	float end_red   = intVals["ERED"] / 256.f;
	float end_green = intVals["EGREEN"] / 256.f;
	float end_blue  = intVals["EBLUE"] / 256.f;
	
	float now_r = end_red   + relativeLife * (start_red   - end_red);
	float now_g = end_green + relativeLife * (start_green - end_green);
	float now_b = end_blue  + relativeLife * (start_blue  - end_blue);
	
	int particlesPerFrame = intVals["PPF"];
	
	Particle p;
	p.r = now_r;
	p.g = now_g;
	p.b = now_b;
	p.a = 1.0f;
	
	p.scale = intVals["SCALE"] / 1000.0f;
	p.max_life = intVals["PLIFE"];
	p.cur_life = p.max_life;
	
	p.pos = position;
	p.target_pos = position;
	
	FixedPoint max_var(intVals["PSP_1000"], 1000);
	max_var *= FixedPoint(intVals["CUR_LIFE"], intVals["MAX_LIFE"]);
	FixedPoint half_var = max_var / FixedPoint(2);
	
	
	for(int i=0; i<particlesPerFrame; ++i)
	{
		p.vel = velocity;
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
	
	--intVals["CUR_LIFE"];
}



bool ParticleSource::alive() const
{
	return intVals.find("CUR_LIFE")->second > 0;
}

