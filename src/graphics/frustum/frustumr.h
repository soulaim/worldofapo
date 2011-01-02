#ifndef FRUSTUMR_H
#define FRUSTUMR_H

#include "vec3.h"
#include "plane.h"

class FrustumR
{
public:
	void setCamInternals(float angle, float radius, float nearD, float farD);
	void setCamDef(const Vec3& p, const Vec3& l, const Vec3& u);

	enum FrustumResult { OUTSIDE, INTERSECT, INSIDE };
	FrustumResult pointInFrustum(const Vec3& p) const;
	FrustumResult sphereInFrustum(const Vec3& p, float radius) const;

	Vec3 ntl,ntr,nbl,nbr;
	Vec3 ftl,ftr,fbl,fbr;
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

	Vec3 X,Y,Z;
	Vec3 camPos;
	float near, far, ratio, angle;
	float sphereFactorX, sphereFactorY;
	double tang;
	double nw,nh,fw,fh;
};


#endif
