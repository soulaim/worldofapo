#include "../fixed_point.h"
#include "../apomath.h"

#include <cassert>
#include <iostream>

using namespace std;

int main()
{
	FixedPoint p1 = 2;
	FixedPoint p2 = 4;

	FixedPoint p3 = p1 * p2;
	assert(p3.getInteger() == 8);

	FixedPoint p4 = p1;
	p4 *= p2;
	assert(p4.getInteger() == 8);

	FixedPoint p5 = 8;
	p5 /= p2;
	assert(p5.getInteger() == 2);

	assert(ApoMath::sqrt(FixedPoint(4)).getInteger() == 2);
	assert(ApoMath::sqrt(FixedPoint(1)).getInteger() == 1);
	assert((ApoMath::sqrt(FixedPoint(1)/FixedPoint(4)) * FixedPoint(2)).getInteger() == 1);
}

