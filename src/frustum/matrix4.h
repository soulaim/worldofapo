#ifndef MATRIX4_H
#define MATRIX4_H

#include <ostream>

#include "Vec3.h"

class Matrix4
{
public:
	float T[16];


	Matrix4();
	Matrix4(float rotx, float roty, float rotz, float dx, float dy, float dz);

	Matrix4& operator*=(const Matrix4& v);
	Matrix4 operator *(const Matrix4& v) const;

	Vec3 operator *(const Vec3& v) const;
};

#endif

