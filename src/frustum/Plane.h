#ifndef PLANE_H
#define PLANE_H

#include "Vec3.h"

class Plane
{
	Vec3 point;
	float d;

public:
	Vec3 normal;

	Plane(const Vec3& v1, const Vec3& v2, const Vec3& v3);
	Plane(void);
	~Plane();

	void set3Points(const Vec3& v1, const Vec3& v2, const Vec3& v3);
	void setNormalAndPoint(const Vec3& normal, const Vec3& point);
	void setCoefficients(float a, float b, float c, float d);
	float distance(const Vec3& p);

	void print();

};

#endif

