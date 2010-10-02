
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
		FixedPoint length = ApoMath::sqrt(x*x + y*y);
		
		if(length.number == 0)
			return;
		x /= length;
		y /= length;
	}
	
	void operator*=(const FixedPoint& scalar)
	{
		x *= scalar;
		y *= scalar;
	}
	
	void operator += (const Location& a)
	{
		x += a.x;
		y += a.y;
	}
	
	void operator -= (const Location& a)
	{
		x -= a.x;
		y -= a.y;
	}
};

#endif
