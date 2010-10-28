#include "location.h"

Location::Location():
	x(FixedPoint(0)),
	y(FixedPoint(0)),
	z(FixedPoint(0))
{
}

Location::Location(const FixedPoint& a, const FixedPoint& b, const FixedPoint& c):
	x(a),
	y(b),
	z(c)
{
}

Location Location::operator*(const FixedPoint& scalar) const
{
	return Location(*this) *= scalar;
}

Location Location::operator/(const FixedPoint& scalar) const
{
	return Location(*this) /= scalar;
}

Location& Location::operator*=(const FixedPoint& scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

Location& Location::operator/=(const FixedPoint& scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
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

FixedPoint Location::lengthSquared() const
{
	return (x * x + y * y + z * z);
}

std::ostream& operator<<(std::ostream& out, const Location& loc)
{
	return out << "(" << loc.x << "," << loc.y << "," << loc.z << ")";
}

bool operator==(const Location& a, const Location& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!=(const Location& a, const Location& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

