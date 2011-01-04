#ifndef TEST_H
#define TEST_H

#include <cassert>
#include <iostream>
#include "graphics/frustum/vec3.h"

const float epsilon = 0.0001;

template<typename T>
bool equal(const T& a, const T& b)
{
	return a == b;
}

bool equal(const Vec3& a, const Vec3& b)
{
	return (a - b).lengthSquared() < epsilon;
}

template<typename T>
void checkEqual(const T& a, const T& b, int line)
{
	if(!equal(a, b))
	{
		std::cout << "FAIL line " << line << ": got " << a << " != " << b << " expected.\n";
	}
}

#define assertEqual(a,b) checkEqual(a,b, __LINE__)

#endif

