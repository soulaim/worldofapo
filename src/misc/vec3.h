
#ifndef H_LOCATION__
#define H_LOCATION__

#include "fixed_point.h"
#include "apomath.h"

#include <iostream>

template <class T>
class vec3
{
public:
	vec3();
	vec3(const T& a, const T& b, const T& c);

	T x, y, z;

	inline void normalize();
	vec3<T>& operator*=(const T& scalar);
	vec3<T>& operator/=(const T& scalar);
	vec3<T>& operator+=(const vec3<T>&);
	vec3<T>& operator-=(const vec3<T>&);
	vec3<T> operator+(const vec3<T>&) const;
	vec3<T> operator-(const vec3<T>&) const;
	vec3<T> operator*(const vec3<T>&) const;

	vec3<T> operator*(const T& scalar) const;
	vec3<T> operator/(const T& scalar) const;
	vec3<T>& set(const T&, const T&, const T&);

	T operator[](int i) const;

	vec3 crossProduct(const vec3<T>&) const;
	T dotProduct(const vec3<T>& b) const;
	T length() const;
	T lengthSquared() const;
};

#include "vec3.icc"

template <class T> vec3<T> operator-(const vec3<T>& a)
{
	return vec3<T>(-a.x, -a.y, -a.z);
}

template <class T> bool operator == (const vec3<T>& a, const vec3<T>& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

template <class T> bool operator != (const vec3<T>& a, const vec3<T>& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

template <class T> std::ostream& operator<<(std::ostream& out, const vec3<T>& loc)
{
	return out << "(" << loc.x << "," << loc.y << "," << loc.z << ")";
}

template <class T> void vec3<T>::normalize()
{
	T length = squareRoot(x*x + y*y + z*z);

	x /= length;
	y /= length;
	z /= length;
}

typedef vec3<FixedPoint> Location;

#endif
