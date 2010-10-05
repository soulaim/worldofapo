#include "location.h"

void Location::normalize()
{
	FixedPoint length = ApoMath::sqrt(x*x + y*y + z*z);
	
	if(length.number == 0)
		return;
	x /= length;
	y /= length;
	z /= length;
}

Location& Location::operator*=(const FixedPoint& scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

Location& Location::operator+=(const Location& a)
{
	x += a.x;
	y += a.y;
	z += a.z;
	return *this;
}
Location& Location::operator-=(const Location& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
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
	result.x = y * b.z  -  z * b.y;
	result.y = z * b.x  -  x * b.z;
	result.z = x * b.y  -  y * b.x;
	return result;
}

FixedPoint Location::dotProduct(const Location& b) const
{
	FixedPoint result;
	result = x * b.x + y * b.y + z * b.z;
	return result;
}

FixedPoint Location::length() const
{
	return (x * x + y * y + z * z).squareRoot();
}



