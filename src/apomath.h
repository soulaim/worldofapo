
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
	void clamp(int& angle) const;
	FixedPoint& getCos(int& angle) const;
	FixedPoint& getSin(int& angle) const;
	float getDegrees(int& angle) const;

	static FixedPoint sqrt(const FixedPoint& point);

	ApoMath();
	void init(int);
	bool ready() const;


	static int DEGREES_90;
	static int DEGREES_180;
	static int DEGREES_360;
};

#endif
