
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

void Unit::tick()
{
  // if unit is ok, it should move towards its destination
  

  if(keyState & 1) // if should be moving, moves forward
  {
    position.y += apomath.getSin(angle);
    position.x += apomath.getCos(angle);
  }
}

