#ifndef H_ALGORITHMS
#define H_ALGORITHMS

#include "location.h"
#include "frustum/vec3.h"

#include <array>

// Given a line segment (a, b), checks if point p is inside the segment.
// Assumes p is a point along the line defined by a & b.
FixedPoint pointInLinesegment(const Location& a, const Location& b, const Location& p);

// Given a plane by the three 3d-coordinates (A, B, C), calculates the y-coordinate on the plane
// of the given xz-point p (which lies usually inside the triangle ABC).
void interpolate(const Location& A, const Location& B, const Location& C, Location& p);


// Given a light's relative position to the camera, the maximum radius that the light affects,
// and the size of the screen in pixels, calculates the area of the screen that the light
// might affect. Returns the size of the affected area or 0 if the light can't affect any
// pixel on the screen.
int light_scissor(const Vec3& lightpos, float r, int screen_width, int screen_height, std::array<int,4>& rect );

#endif

