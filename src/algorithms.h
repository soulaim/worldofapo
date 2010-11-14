#ifndef H_ALGORITHMS
#define H_ALGORITHMS

#include "location.h"

// Given a line segment (a, b), checks if point p is inside the segment.
// Assumes p is a point along the line defined by a & b.
FixedPoint pointInLinesegment(const Location& a, const Location& b, const Location& p);

// Given a plane by the three 3d-coordinates (A, B, C), calculates the y-coordinate on the plane
// of the given xz-point p (which lies usually inside the triangle ABC).
void interpolate(const Location& A, const Location& B, const Location& C, Location& p);

#endif

