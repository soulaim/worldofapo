
#include "Vec3.h"

#include <cmath>
#include <cstdio>

using namespace std;

Vec3::Vec3(float xx, float yy, float zz):
	x(xx),
	y(yy),
	z(zz)
{
}

Vec3::Vec3(const Vec3 &v):
	x(v.x),
	y(v.y),
	z(v.z)
{
}

Vec3::Vec3():
	x(0.0f),
	y(0.0f),
	z(0.0f)
{
}


Vec3::~Vec3()
{
}

Vec3& Vec3::operator=(const Vec3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

Vec3& Vec3::operator+=(const Vec3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vec3& Vec3::operator-=(const Vec3 &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vec3& Vec3::operator*=(float t)
{
	x *= t;
	y *= t;
	z *= t;
	return *this;
}

Vec3& Vec3::operator/=(float t)
{
	x /= t;
	y /= t;
	z /= t;
	return *this;
}


Vec3 Vec3::operator+(const Vec3 &v) const
{
	return Vec3(*this) += v;
}

Vec3 Vec3::operator-(const Vec3 &v) const
{
	return Vec3(*this) -= v;
}

Vec3 Vec3::operator*(float t) const
{
	return Vec3(*this) *= t;
}

Vec3 Vec3::operator/(float t) const
{
	return Vec3(*this) /= t;
}



Vec3 Vec3::operator-(void) const
{
	return Vec3(-x, -y, -z);
}

// cross product
Vec3 Vec3::operator*(const Vec3 &v) const
{
	Vec3 res;

	res.x = y * v.z - z * v.y;
	res.y = z * v.x - x * v.z;
	res.z = x * v.y - y * v.x;

	return (res);
}

float Vec3::length() const
{
	return((float)sqrt(x*x + y*y + z*z));
}

float Vec3::lengthSquared() const
{
	return x*x + y*y + z*z;
}

void Vec3::normalize()
{
	float len = length();
	if (len)
	{
		x /= len;;
		y /= len;
		z /= len;
	}
}


float Vec3::innerProduct(const Vec3 &v) const
{
	return (x * v.x + y * v.y + z * v.z);
}

void Vec3::set(float x,float y, float z) {

	this->x = x;
	this->y = y;
	this->z = z;
}

// -----------------------------------------------------------------------
// For debug pruposes: Prints a vector
// -----------------------------------------------------------------------
void Vec3::print() {
	printf("Vec3(%f, %f, %f)",x,y,z);
	
}

float Vec3::operator[](int i) const
{
	switch(i)
	{
		case 0: return x;
		case 1: return y;
		case 2: return z;
	}
	return 1.0f;
}

