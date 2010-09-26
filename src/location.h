
#ifndef H_LOCATION__
#define H_LOCATION__

#include "fixed_point.h"

struct Location
{
  Location():x(0), y(0), h(0) {}
  FixedPoint x;
  FixedPoint y;
  FixedPoint h;
  
  void normalize()
  {
    FixedPoint sum = x.abs() + y.abs();
    if(sum.number == 0)
      return;
    
    x /= sum;
    y /= sum;
  }
  
  void operator -= (const Location& a)
  {
    x -= a.x;
    y -= a.y;
  }
  
};

#endif
