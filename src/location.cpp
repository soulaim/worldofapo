#include "location.h"

bool Location::near(const Location& location, const FixedPoint& distance) const
{
	FixedPoint dx = (x - location.x);
	FixedPoint dy = (y - location.y);
	FixedPoint dh = (h - location.h);

	return dx*dx + dy*dy + dh*dh <= distance*distance;
}

void Location::normalize()
{
	FixedPoint length = ApoMath::sqrt(x*x + y*y + h*h);
	
	if(length.number == 0)
		return;
	x /= length;
	y /= length;
	h /= length;
}

Location& Location::operator*=(const FixedPoint& scalar)
{
	x *= scalar;
	y *= scalar;
	h *= scalar;
	return *this;
}

Location& Location::operator+=(const Location& a)
{
	x += a.x;
	y += a.y;
	h += a.h;
	return *this;
}
Location& Location::operator-=(const Location& a)
{
	x -= a.x;
	y -= a.y;
	h -= a.h;
	return *this;
}


Location Location::operator+(const Location& b) const
{
	return Location(*this) += b;
}
Location Location::operator-(const Location& b) const
{
	return Location(*this) -= b;
}


Location Location::crossProduct(const Location& b) const
{
	Location result;
	result.x = h * b.y  -  y * b.h;
	result.h = y * b.x  -  x * b.y;
	result.y = x * b.h  -  h * b.x;
	return result;
}

FixedPoint Location::dotProduct(const Location& b) const
{
	FixedPoint result;
	result = x * b.x + h * b.h + y * b.y;
	return result;
}

FixedPoint Location::length() const
{
	return (x * x + h * h + y * y).squareRoot();
}



