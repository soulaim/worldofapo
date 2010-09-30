
#include "unit.h"
#include <cmath>
#include <iostream>

using std::atan2;


Unit::Unit(): angle(0), keyState(0)
{
}

float Unit::getAngle(ApoMath& apomath)
{
  return apomath.getRad(angle);
}

void Unit::updateInput(int keyState_, int mousex_, int mousey_)
{
//  keyState ^= keyState_;
  keyState = keyState_;
  angle += mousex_;
}

void Unit::tick(ApoMath& apomath, int multiplier)
{

  // if unit is ok, it should move towards its destination
  if(keyState & 4) // if should be moving, moves forward
  {
    position.y.number += multiplier * apomath.getSin(angle).number / 1000;
    position.x.number += multiplier * apomath.getCos(angle).number / 1000;
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
}

