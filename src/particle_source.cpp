
#include "particle_source.h"

using namespace std;


// TODO: this should be a bit smarter maybe? oh well.
void getColor(const string& a, int& r, int& g, int& b)
{
	if(a == "WHITE")
	{
		r = 255;
		g = 255;
		b = 255;
		return;
	}
	else if(a == "ORANGE")
	{
		r = 255;
		g = 165;
		b = 0;
		return;
	}
	else if(a == "DARK_RED")
	{
		r = 100;
		g = 0;
		b = 0;
		return;
	}
	else
	{
		r = 0;
		g = 255;
		b = 255;
		return;
	}
}


void ParticleSource::setColors(const string& scstart, const string& scend, const string& ecstart, const string& ecend)
{
	getColor( scstart, ssred, ssgreen, ssblue );
	getColor( scend  , sered, segreen, seblue );
	
	getColor( ecstart  , esred, esgreen, esblue );
	getColor( ecend    , eered, eegreen, eeblue );
}

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
	
	// this should usually be in ParticleSource.tick() probably
	int cs_red   = esred   + (ssred - esred)     * relativeLife;
	int cs_green = esgreen + (ssgreen - esgreen) * relativeLife;
	int cs_blue  = esblue  + (ssblue - esblue)   * relativeLife;
	
	// these values are never used
	int ce_red   = eered   + (sered - eered)     * relativeLife;
	int ce_green = eegreen + (segreen - eegreen) * relativeLife;
	int ce_blue  = eeblue  + (seblue - eeblue)   * relativeLife;
	
	Particle p;
	p.sr = cs_red   / 256.f;
	p.sg = cs_green / 256.f;
	p.sb = cs_blue  / 256.f;
	
	p.er = ce_red   / 256.f;
	p.eg = ce_green / 256.f;
	p.eb = ce_blue  / 256.f;
	
	/*
	float now_r = end_red   + relativeLife * (start_red   - end_red);
	float now_g = end_green + relativeLife * (start_green - end_green);
	float now_b = end_blue  + relativeLife * (start_blue  - end_blue);
	*/
	
	int particlesPerFrame = intVals["PPF"];
	
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

