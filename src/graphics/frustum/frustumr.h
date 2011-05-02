#ifndef FRUSTUMR_H
#define FRUSTUMR_H

#include "misc/vec3.h"
#include "plane.h"

#ifdef _WIN32
#undef near
#undef far
#endif

class FrustumR
{
public:
	void setCamInternals(float angle, float radius, float nearD, float farD);
	void setCamDef(const vec3<float>& p, const vec3<float>& l, const vec3<float>& u);

	enum FrustumResult { OUTSIDE, INTERSECT, INSIDE };
	FrustumResult pointInFrustum(const vec3<float>& p) const;
	FrustumResult sphereInFrustum(const vec3<float>& p, float radius) const;

	vec3<float> ntl,ntr,nbl,nbr;
	vec3<float> ftl,ftr,fbl,fbr;
private:

	enum
	{
		TOP = 0,
		BOTTOM,
		LEFT,
		RIGHT,
		NEARP,
		FARP
	};

	void setFrustum(float* m);

	Plane plane[6];

	vec3<float> X,Y,Z;
	vec3<float> camPos;
	float near;
	float far;
	float ratio;
	float angle;

	float sphereFactorX, sphereFactorY;
	double tang;
	double nw,nh,fw,fh;
};


#endif
