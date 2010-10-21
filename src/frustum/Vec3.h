#ifndef VEC3_H
#define VEC3_H

#include <ostream>

class Vec3
{

public:

	float x,y,z;


	Vec3(float x, float y, float z);
	Vec3(const Vec3 &v);
	Vec3();
	~Vec3();
	Vec3& operator=(const Vec3&);

	Vec3& operator+=(const Vec3 &v);
	Vec3& operator-=(const Vec3 &v);
	Vec3& operator*=(float t);
	Vec3& operator/=(float t);
	Vec3 operator +(const Vec3 &v) const;
	Vec3 operator -(const Vec3 &v) const;
	Vec3 operator *(float t) const;
	Vec3 operator /(float t) const;

	Vec3 operator *(const Vec3 &v) const;
	Vec3 operator -() const;
	float operator[](int i) const;

	float length() const;
	float lengthSquared() const;
	void normalize();
	float innerProduct(const Vec3& v) const;
	void set(float x, float y, float z);

	void print();

};

inline std::ostream& operator<<(std::ostream& out, const Vec3& vec)
{
	return out << "(" << vec.x << " , " << vec.y << " , " << vec.z << ")";
}

#endif

