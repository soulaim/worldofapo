
#ifndef APOMATH_H
#define APOMATH_H

#include <vector>
#include <cmath>

#include "fixed_point.h"

struct ApoMath
{
  std::vector<FixedPoint> sin_vals;
  std::vector<FixedPoint> cos_vals;
  std::vector<float> rad_vals;
  
  FixedPoint& getCos(int& angle);
  FixedPoint& getSin(int& angle);
  float getRad(int& angle); // actually returns the degrees, not the rads. for openGL.

	static FixedPoint sqrt(const FixedPoint& point);

  void init(int);
};

#endif
