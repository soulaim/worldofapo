
#include "Vec3.h"

#include <cmath>
#include <cstdio>

using namespace std;

Vec3::Vec3(float x, float y, float z)
{

	this->x = x;
	this->y = y;
	this->z = z;

}

Vec3::Vec3(const Vec3 &v) {

	x = v.x;
	y = v.y;
	z = v.z;

}


Vec3::Vec3() {

	x = 0;
	y = 0;
	z = 0;

}


Vec3::~Vec3()
{

}


Vec3& Vec3::operator += (const Vec3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	
	return *this;
}


Vec3 Vec3::operator+(const Vec3 &v) const
{

	Vec3 res;

	res.x = x + v.x;
	res.y = y + v.y;
	res.z = z + v.z;

	return(res);
}

Vec3 Vec3::operator-(const Vec3 &v) const
{

	Vec3 res;

	res.x = x - v.x;
	res.y = y - v.y;
	res.z = z - v.z;

	return(res);
}

Vec3 Vec3::operator-(void) const
{

	Vec3 res;

	res.x = -x;
	res.y = -y;
	res.z = -z;

	return(res);
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

Vec3 Vec3::operator*(float t) const
{

	Vec3 res;

	res.x = x * t;
	res.y = y * t;
	res.z = z * t;

	return (res);
}


Vec3 Vec3::operator/(float t) const
{

	Vec3 res;

	res.x = x / t;
	res.y = y / t;
	res.z = z / t;

	return (res);
}



float Vec3::length() {

	return((float)sqrt(x*x + y*y + z*z));
}

void Vec3::normalize() {

	float len;

	len = length();
	if (len) {
		x /= len;;
		y /= len;
		z /= len;
	}
}


float Vec3::innerProduct(const Vec3 &v) {

	return (x * v.x + y * v.y + z * v.z);
}

Vec3& Vec3::operator=(const Vec3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
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
