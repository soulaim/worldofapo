
#include "lightsource.h"

using namespace std;

LightSource::LightSource()
{
	lightActive = false; // default state of a light emitter is OFF
	linear_attenuation    = 0.0f;
	quadratic_attenuation = 0.0002f;
	
	for(int i=0; i<4; i++)
		c_diffuse_start[i] = 0.f;

	life_maximum = 0;
	life_current = 0;
	power_maximum = 0;
	behaviour = ONLY_DIE;
	unitBind = -1; // not tied to any unit by default.
	lifeType = MORTAL;
}

const FixedPoint& LightSource::getIntensity() const
{
	return intensity;
}

void LightSource::updateIntensity()
{
	float r=0.f, g=0.f, b=0.f;
	getDiffuse(r, g, b);
	intensity = FixedPoint(1000 * (0.299 * r + 0.587 * g + 0.114 * b), 1);
}

void LightSource::setDiffuse(float r, float g, float b)
{
	c_diffuse_start[0] = r;
	c_diffuse_start[1] = g;
	c_diffuse_start[2] = b;
}

void LightSource::setSpecular(float r, float g, float b)
{
	c_specular[0] = r;
	c_specular[1] = g;
	c_specular[2] = b;
}

void LightSource::getDiffuse(float& r, float& g, float& b) const
{
	// TODO: More comples changes of light colour with life source life time
	if(behaviour == RISE_AND_DIE)
	{
		float countDown = float(life_current) / float(life_maximum);
		float mul = (countDown - 1) * (countDown - 1) * countDown * countDown * 8;
		
		r = c_diffuse_start[0] * mul;
		g = c_diffuse_start[1] * mul;
		b = c_diffuse_start[2] * mul;
	}
	else if(behaviour == ONLY_DIE)
	{
		float countDown = float(life_current) / float(life_maximum);
		float mul = countDown;
		
		r = c_diffuse_start[0] * mul;
		g = c_diffuse_start[1] * mul;
		b = c_diffuse_start[2] * mul;
	}
	else if(behaviour == CONSTANT)
	{
		r = c_diffuse_start[0];
		g = c_diffuse_start[1];
		b = c_diffuse_start[2];
	}
}

void LightSource::getSpecular(float& r, float& g, float& b) const
{
	r = g = b = 1.0f;
	
	/*
	float mul = float(life_current) / float(life_maximum);
	
	r = colourValues_specular[0] * mul;
	g = colourValues_specular[1] * mul;
	b = colourValues_specular[2] * mul;
	*/
}


void LightSource::setLife(int max_life)
{
	life_maximum = max_life;
	life_current = max_life;
}

void LightSource::activateLight()
{
	lightActive = true;
}

void LightSource::deactivateLight()
{
	lightActive = false;
}

bool LightSource::tickLight()
{
	childTick();
	
	if(--life_current < 0)
	{
		if(lifeType == MORTAL)
			return false;
		else
			life_current = life_maximum;
	}
	return true;
}


void LightSource::setPower(int pow)
{
	power_maximum = pow;
}
