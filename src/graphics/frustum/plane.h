#ifndef PLANE_H
#define PLANE_H

#include "vec3.h"

class Plane
{
public:
	Vec3 normal;

	Plane(const Vec3& v1, const Vec3& v2, const Vec3& v3);
	Plane();

	void set3Points(const Vec3& v1, const Vec3& v2, const Vec3& v3);
	void setNormalAndPoint(const Vec3& normal, const Vec3& point);
	void setCoefficients(float a, float b, float c, float d);

	float distance(const Vec3& p) const;
private:
	Vec3 point;
	float d;
};

#endif

