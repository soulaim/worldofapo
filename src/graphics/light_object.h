
#ifndef H_LIGHT_OBJECT
#define H_LIGHT_OBJECT

#include "graphics/lightsource.h"
#include "physics/movable_object.h"

class LightObject : public LightSource, public MovableObject
{
public:
	LightObject()
	{
	}

	const Location& getPosition() const
	{
		return MovableObject::getPosition();
	}
	
	void childTick()
	{
		position += velocity;
	}
};

#endif

