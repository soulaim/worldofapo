
#ifndef H_LOCATION__
#define H_LOCATION__

#include "fixed_point.h"
#include "apomath.h"

#include <iostream>

struct Location
{

	Location();
	Location(const FixedPoint& a, const FixedPoint& b, const FixedPoint& c);
	
	FixedPoint x;
	FixedPoint y;
	FixedPoint z;
	
	inline void normalize();
	Location& operator*=(const FixedPoint& scalar);
	Location& operator/=(const FixedPoint& scalar);
	Location& operator+=(const Location&);
	Location& operator-=(const Location&);
	Location operator+(const Location&) const;
	Location operator-(const Location&) const;
	Location operator*(const FixedPoint& scalar) const;
	Location operator/(const FixedPoint& scalar) const;
	
	Location crossProduct(const Location&) const;
	FixedPoint dotProduct(const Location& b) const;
	FixedPoint length() const;
	FixedPoint lengthSquared() const;
};

bool operator==(const Location&, const Location&);
bool operator!=(const Location&, const Location&);

std::ostream& operator<<(std::ostream& out, const Location& loc);

void Location::normalize()
{
	FixedPoint length = (x*x + y*y + z*z).squareRoot();
	
	assert(length > FixedPoint(0));
	
	x /= length;
	y /= length;
	z /= length;
}

#endif
