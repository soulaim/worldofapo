
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
	
	void normalize()
	{
		FixedPoint length = ApoMath::sqrt(x*x + y*y + h*h);
		
		if(length.number == 0)
			return;
		x /= length;
		y /= length;
		h /= length;
	}

	Location& operator*=(const FixedPoint& scalar)
	{
		x *= scalar;
		y *= scalar;
		h *= scalar;
		return *this;
	}

	Location& operator+=(const Location& a)
	{
		x += a.x;
		y += a.y;
		h += a.h;
		return *this;
	}
	Location& operator-=(const Location& a)
	{
		x -= a.x;
		y -= a.y;
		h -= a.h;
		return *this;
	}


	Location operator+(const Location& b) const
	{
		return Location(*this) += b;
	}
	Location operator-(const Location& b) const
	{
		return Location(*this) -= b;
	}
};

inline std::ostream& operator<<(std::ostream& out, const Location& loc)
{
	return out << "(" << loc.x << " , " << loc.h << " , " << loc.y << ")";
}

#endif

