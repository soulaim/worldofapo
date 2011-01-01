#ifndef MATRIX4_H
#define MATRIX4_H

#include <ostream>

#include "vec3.h"

class Matrix4
{
public:
	float T[16];


	Matrix4();
	Matrix4(const Vec3& row1, const Vec3& row2, const Vec3& row3);
	Matrix4(float rotx_degrees, float roty_degrees, float rotz_degrees, float dx, float dy, float dz);

	Matrix4& operator*=(const Matrix4& v);
	Matrix4 operator *(const Matrix4& v) const;

	Vec3 operator *(const Vec3& v) const;

	static Matrix4 projectionFrustum(float left, float right, float bottom, float top, float near, float far);
};

std::ostream& operator<<(std::ostream& out, const Matrix4& m);

#endif

