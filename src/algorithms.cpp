#include "algorithms.h"

FixedPoint pointInLinesegment(const Location& a, const Location& b, const Location& p)
{
	FixedPoint segmentLength = (b - a).dotProduct(b - a);
	return (a-p).dotProduct(b-p) / segmentLength;
}

void interpolate(const Location& A, const Location& B, const Location& C, Location& p)
{
	Location direction1 = B - A;
	Location direction2 = C - A;

	// A + t*(direction1) + u*(direction2) = p;
	//
	// or in other words:
	//
	// t*(direction11.x) + u*(direction2.x) = p.x - A.x
	// t*(direction11.z) + u*(direction2.z) = p.z - A.z
	//
	// Solution for t = t_top/t_bot and u = u_top / u_bot is:
	FixedPoint t_top = direction2.x * (p.z - A.z) - direction2.z * (p.x - A.x);
	FixedPoint u_top = direction1.x * (p.z - A.z) - direction1.z * (p.x - A.x);
	FixedPoint t_bot = direction2.x * direction1.z - direction1.x * direction2.z;
	FixedPoint u_bot = direction1.x * direction2.z - direction2.x * direction1.z;

	p = A + direction1 * t_top / t_bot + direction2 * u_top / u_bot;
}

