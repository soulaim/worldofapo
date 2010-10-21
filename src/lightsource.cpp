
#include "lightsource.h"

using namespace std;

LightSource::LightSource()
{
	lightActive = false; // default state of a light emitter is OFF
	
	colourValues_diffuse.resize(4, 0.f);
	colourValues_specular.resize(4, 0.f);
	
	linear_attenuation    = 0.0f;
	quadratic_attenuation = 0.0002f;
	
	life_maximum = 0;
	life_current = 0;
	power_maximum = 0;
}

const FixedPoint& LightSource::getIntensity()
{
	intensity = FixedPoint(life_current, life_maximum) * power_maximum;
	return intensity;
}

void LightSource::setDiffuse(float r, float g, float b)
{
	colourValues_diffuse[0] = r;
	colourValues_diffuse[1] = g;
	colourValues_diffuse[2] = b;
}

void LightSource::setSpecular(float r, float g, float b)
{
	colourValues_specular[0] = r;
	colourValues_specular[1] = g;
	colourValues_specular[2] = b;
}

void LightSource::getDiffuse(float& r, float& g, float& b) const
{
	float mul = float(life_current) / float(life_maximum);
	
	r = colourValues_diffuse[0] * mul;
	g = colourValues_diffuse[1] * mul;
	b = colourValues_diffuse[2] * mul;
}

void LightSource::getSpecular(float& r, float& g, float& b) const
{
	float mul = float(life_current) / float(life_maximum);
	
	r = colourValues_specular[0] * mul;
	g = colourValues_specular[1] * mul;
	b = colourValues_specular[2] * mul;
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
	if(--life_current < 0)
		return false;
	return true;
}


void LightSource::setPower(int pow)
{
	power_maximum = pow;
}
