
#include "unit.h"
#include <cmath>
#include <iostream>

using std::atan2;


Unit::Unit():
	angle(0),
	upangle(0),
	keyState(0),
	weapon_cooldown(0)
{
}

float Unit::getAngle(ApoMath& apomath)
{
	return apomath.getRad(angle);
}

float Unit::getUpAnle(ApoMath& apomath)
{
	return apomath.getRad(upangle);
}

void Unit::updateInput(int keyState_, int mousex_, int mousey_)
{
	//  keyState ^= keyState_;
	keyState = keyState_;
	angle -= mousex_;
	upangle -= mousey_;
}


bool Unit::movingFront()
{
	return (keyState & (1 << 22)) || (keyState & (1 << 2));
}

bool Unit::movingBack()
{
	return ((keyState & (1 << 23))) || (keyState & (1 << 3));
}

bool Unit::movingLeft()
{
	return (keyState & (1 << 20));
}

bool Unit::movingRight()
{
	return (keyState & (1 << 21));
}

bool Unit::shooting()
{
	return keyState & (1 << 24);
}

/*
void Unit::tick(ApoMath& apomath, int multiplier)
{
	
	// if unit is ok, it should move towards its destination
	if(keyState & 4) // if should be moving, moves forward
	{
		position.y.number += multiplier * apomath.getSin(angle).number / 1000;
		position.x.number += multiplier * apomath.getCos(angle).number / 1000;
		}
		
		if(keyState & 1) // if should be moving, turns left
		{
			angle += 2;
		}
		if(keyState & 2) // if should be moving, turns right
		{
			angle -= 2;
		}
		}
		*/
