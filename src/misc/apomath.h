
#ifndef APOMATH_H
#define APOMATH_H

#include <vector>
#include <cmath>

#include "fixed_point.h"

struct ApoMath
{
private:
	static std::vector<FixedPoint> sin_vals;
	static std::vector<FixedPoint> cos_vals;
	static std::vector<float> degree_vals;

public:
	static void clamp(int& angle);
	static FixedPoint& getCos(int& angle);
	static FixedPoint& getSin(int& angle);
	static float getDegrees(int& angle);

	static FixedPoint sqrt(const FixedPoint& point);

	ApoMath();
	static void init(int);

	static int DEGREES_90;
	static int DEGREES_180;
	static int DEGREES_360;
};

#endif
