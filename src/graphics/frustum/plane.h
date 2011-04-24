#ifndef PLANE_H
#define PLANE_H

#include "vec3.h"

class Plane
{
public:
	vec3<float> normal;

	Plane(const vec3<float>& v1, const vec3<float>& v2, const vec3<float>& v3);
	Plane();

	void set3Points(const vec3<float>& v1, const vec3<float>& v2, const vec3<float>& v3);
	void setNormalAndPoint(const vec3<float>& normal, const vec3<float>& point);
	void setCoefficients(float a, float b, float c, float d);

	float distance(const vec3<float>& p) const;
private:
	vec3<float> point;
	float d;
};

#endif

