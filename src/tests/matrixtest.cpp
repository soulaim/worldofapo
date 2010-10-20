#include "../frustum/matrix4.h"

#include <cassert>
#include <iostream>
#include <cmath>

using namespace std;

const float epsilon = 0.0001;

bool equal(const Vec3& a, const Vec3& b)
{
	return (a - b).lengthSquared() < epsilon;
}

template<typename T>
void checkEqual(const T& a, const T& b, int line)
{
	if(!equal(a, b))
	{
		cout << "FAIL line " << line << ": got " << a << " != " << b << " expected.\n";
	}
}

#define assertEqual(a,b) checkEqual(a,b, __LINE__)

int main()
{
	{
		Matrix4 m;
		Vec3 v1(4, 3, 2);
		Vec3 v2 = m*v1;
		assertEqual(v1, v2);
	}

	// Rotate around X axis.
	{
		Matrix4 m(M_PI, 0, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(2, 0, 0));
	}

	// Rotate around Y axis.
	{
		Matrix4 m(0, M_PI, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(-2, 0, 0));
	}
	{
		Matrix4 m(0, M_PI/2, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(0, 0, -2));
	}
	{
		Matrix4 m(0, -M_PI/2, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(0, 0, 2));
	}

	// Rotate around Z axis.
	{
		Matrix4 m(0, 0, M_PI, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(-2, 0, 0));
	}
	{
		Matrix4 m(0, 0, M_PI/2, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(0, 2, 0));
	}
	{
		Matrix4 m(0, 0, -M_PI/2, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(0, -2, 0));
	}

	// Matrix * Matrix
	{
		Matrix4 m1;
		Matrix4 m2;
		float M1[16] = {
			1, 4, 6, 10,
			2, 7, 5, 3,
			1, 2, 3, 4,
			6, 7, 8, 9 };
		float M2[16] = {
			1, 4,  6, 0,
			2, 7,  5, 0,
			9, 0, 11, 0,
			3, 1,  0, 1 };
		float M3[16] = {
			93, 42, 92, 10,
			70, 60, 102, 3,
			44, 22, 49, 4,
			119, 82, 159, 9 };
		for(int i = 0; i < 16; ++i)
		{
			m1.T[i] = M1[i];
			m2.T[i] = M2[i];
		}
		m1 *= m2;

		for(int i = 0; i < 16; ++i)
		{
			if(abs(m1.T[i] - M3[i]) > epsilon)
			{
				cout << "Virhe: " << i << "\n";
			}
		}

	}
}


