#include "matrix4.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>

using namespace std;

Matrix4::Matrix4(float rotx, float roty, float rotz, float dx, float dy, float dz)
{
	// http://en.wikipedia.org/wiki/Rotation_matrix#Basic_rotations

	const float conv = 3.14159265f / 180.0f;
	float sx = sin(rotx * conv);
	float sy = sin(roty * conv);
	float sz = sin(rotz * conv);

	float cx = cos(rotx * conv);
	float cy = cos(roty * conv);
	float cz = cos(rotz * conv);

	T[0] = cy * cz;
	T[4] = cy * sz;
	T[8] = -sy;

	T[1] = -cx * sz + sx * sy * cz;
	T[5] =  cx * cz + sx * sy * sz;
	T[9] =  sx * cy;

	T[2] =  sx * sz + cx * sy * cz;
	T[6] = -sx * cz + cx * sy * sz;
	T[10] = cx * cy;

	T[3] = dx;
	T[7] = dy;
	T[11] = dz;

	T[12] = T[13] = T[14] = 0.0f;
	T[15] = 1.0f;
}

Matrix4::Matrix4()
{
	std::memset(T, 0.0f, sizeof(float) * 16);
	T[0] = 1.0f;
	T[5] = 1.0f;
	T[10] = 1.0f;
	T[15] = 1.0f;
}

float tmp[16] = { };

Matrix4& Matrix4::operator*=(const Matrix4 &v)
{
	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			float sum = 0.0;
			for(int k = 0; k < 4; ++k)
			{
				sum += T[i * 4 + k] * v.T[k * 4 + j];
			}
			tmp[i * 4 + j] = sum;
		}
	}
	memcpy(T, tmp, sizeof(float) * 16);

	return *this;
}

Matrix4 Matrix4::operator*(const Matrix4 &v) const
{
	return Matrix4(*this) *= v;
}

Vec3 Matrix4::operator*(const Vec3& v) const
{
	for(int i = 0; i < 3; ++i)
	{
		float sum = 0.0f;
		for(int k = 0; k < 4; ++k)
		{
			sum += T[i*4 + k] * v[k];
		}
		tmp[i] = sum;
	}
	return Vec3(tmp[0], tmp[1], tmp[2]);
}

std::ostream& operator<<(std::ostream& out, const Matrix4& m)
{
	out << "[ ";
	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			out << fixed << setprecision(2) << setw(5) << m.T[i * 4 + j] << " ";
		}
		out << "\n  ";
	}
	out << "]";
	return out;
}

