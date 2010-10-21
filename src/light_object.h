
#ifndef H_LIGHT_OBJECT
#define H_LIGHT_OBJECT

#include "lightsource.h"
#include "movable_object.h"

class LightObject : public LightSource, public MovableObject
{
public:
	LightObject()
	{
	}
	
	const Location& getPosition() const
	{
		return position;
	}
	
};

#endif

