
#include "algorithms.h"

// given a line segment (a, b), checks if point p is inside the segment.
// assumes p is a point along the line defined by a & b
FixedPoint pointInLinesegment(const Location& a, const Location& b, const Location& p)
{
	FixedPoint segmentLength = (b - a).dotProduct(b - a);
	return (a-p).dotProduct(b-p) / segmentLength;
}

