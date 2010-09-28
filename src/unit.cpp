
#include "unit.h"
#include <cmath>
#include <iostream>

using std::atan2;


Unit::Unit(): angle(0), keyState(0)
{
  apomath.init(300);
}

float Unit::getAngle()
{
  return apomath.getRad(angle);
}

void Unit::updateInput(int keyState_, int mousex_, int mousey_)
{
  keyState ^= keyState_;
  angle += mousex_;
}

void Unit::tick()
{
	// if unit is ok, it should move towards its destination


	FixedPoint movespeed = 2;
	if(keyState & 4) // if should be moving, moves forward
	{
		Location delta;
		delta.y = apomath.getSin(angle);
		delta.x = apomath.getCos(angle);

		delta.normalize();
		delta *= movespeed;

		position += delta;
	}
	if(keyState & 8) // if should be moving, moves backward
	{
		position.y -= apomath.getSin(angle);
		position.x -= apomath.getCos(angle);
	}
	if(keyState & 1) // if should be moving, turns left
	{
		angle += 2;
	}
	if(keyState & 2) // if should be moving, turns right
	{
		angle -= 2;
	}

	if(keyState & 32)
	{
		position.setYaw(position.getYaw() + 0.05);
	}
	if(keyState & 64)
	{
		position.setYaw(position.getYaw() - 0.05);
	}
	if(keyState & 128)
	{
		position.setPitch(position.getPitch() + 0.05);
	}
	if(keyState & 256)
	{
		position.setPitch(position.getPitch() - 0.05);
	}
	if(keyState & 512)
	{
		position.setRoll(position.getRoll() + 0.05);
	}
	if(keyState & 1024)
	{
		position.setRoll(position.getRoll() - 0.05);
	}
}

