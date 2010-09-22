
#include "unit.h"
#include <cmath>

using std::cos;
using std::sin;

void Unit::move(float distance)
{
  location.x += distance * cos(3.14159 * -angle / 180.);
  location.y += distance * sin(3.14159 * -angle / 180.);
}

