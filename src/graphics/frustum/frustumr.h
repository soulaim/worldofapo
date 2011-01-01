#ifndef FRUSTUMR_H
#define FRUSTUMR_H

#include "vec3.h"
#include "plane.h"

// Geometric version of Frustum Culling

class FrustumR
{
private:

	enum {
		TOP = 0,
		BOTTOM,
		LEFT,
		RIGHT,
		NEARP,
		FARP
	};


public:

	enum {OUTSIDE, INTERSECT, INSIDE};

	Plane pl[6];

	Vec3 ntl,ntr,nbl,nbr,ftl,ftr,fbl,fbr,X,Y,Z,camPos;
	float nearD, farD, ratio, angle;
	float sphereFactorX, sphereFactorY;
	double tang;
	double nw,nh,fw,fh;

	FrustumR();
	~FrustumR();

	void setFrustum(float *m);
	void setCamInternals(float angle, float radius, float nearD, float farD);
	void setCamDef(Vec3 &p, Vec3 &l, Vec3 &u);
	int pointInFrustum(const Vec3 &p) const;
	int sphereInFrustum(const Vec3 &p, float radius) const;

	void drawPoints();
	void drawLines();
	void drawPlanes();
	void drawNormals();

	void printPlanes();

};


#endif
