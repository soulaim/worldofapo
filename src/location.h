
#ifndef H_LOCATION__
#define H_LOCATION__

#include "fixed_point.h"
#include "apomath.h"

#include <iostream>

struct Location
{
	Location():
		x(0),
		y(0),
		h(0)
	{}
	
	FixedPoint x;
	FixedPoint y;
	FixedPoint h;
	
	bool near(const Location& location, const FixedPoint& distance) const;
	void normalize();
	Location& operator*=(const FixedPoint& scalar);
	Location& operator+=(const Location& a);
	Location& operator-=(const Location& a);
	Location operator+(const Location& b) const;
	Location operator-(const Location& b) const;
};

std::ostream& operator<<(std::ostream& out, const Location& loc);

#endif

