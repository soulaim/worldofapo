
#ifndef H_LIGHTSOURCE
#define H_LIGHTSOURCE

#include <vector>

#include "fixed_point.h"
#include "location.h"
#include "frustum/Vec3.h"

class LightSource
{
public:
	LightSource();
	
	enum
	{
		RISE_AND_DIE = 0,
		ONLY_DIE = 1,
		
		IMMORTAL = 1,
		MORTAL   = 0
	};
	
	virtual const Location& getPosition() const = 0;
	const FixedPoint& getIntensity(); // lights can have an effect on game data, so return value in FixedPoint format.
	
	void setDiffuse(float r, float g, float b);
	void setSpecular(float r, float g, float b);
	
	void getDiffuse(float& r, float& g, float& b) const;
	void getSpecular(float& r, float& g, float& b) const;
	
	void setPower(int pow);
	void setLife(int max_life);
	
	void activateLight();
	void deactivateLight();
	
	bool tickLight();
	
	int unitBind;
	int lifeType;
	int behaviour;
	
	Vec3 drawPos;
	
	// TODO: Copy message for light source information. Structure must retain information about where the light source is tied to.
	
private:
	float c_diffuse_start[4];
	float c_diffuse_end[4];
	float c_specular[4];
	
	float linear_attenuation;
	float quadratic_attenuation;
	
	bool lightActive;
	int life_maximum;
	int life_current;
	
	int power_maximum;
	
	FixedPoint intensity; // member just so we can return references instead of copies.
	// TODO: Add support for spot lights. (might need reworking on shaders also)
	
};

#endif


