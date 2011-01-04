#include "graphics/frustum/matrix4.h"
#include "test.h"

#include <cassert>
#include <iostream>
#include <cmath>

using namespace std;

int main()
{
	// Unit matrix
	{
		Matrix4 m;
		Vec3 v1(4, 3, 2);
		Vec3 v2 = m*v1;
		assertEqual(v1, v2);
	}

	{
		Matrix4 m(0,0,0, 0,0,0);
		Vec3 v1(4, 3, 2);
		Vec3 v2 = m*v1;
		assertEqual(v1, v2);
	}

	// Translation matrix
	{
		Vec3 v0(3, 22, 777);
		Matrix4 m1(12,  0,  0, 0, 0, 0);
		Matrix4 m2(0,  34,  0, 0, 0, 0);
		Matrix4 m3(0,   0, 16, 0, 0, 0);
		Matrix4 m4(0,   0,  0, 1, 2, 3);
		Vec3 v1 = v0;
		v1 = m1 * v1;
		v1 = m2 * v1;
		v1 = m3 * v1;
		v1 = m4 * v1;
		Matrix4 m5(12, 34, 16, 1, 2, 3);
		Vec3 v2 = v0;
		v2 = m5 * v2;
		assertEqual(v2, v1);
	}
	{
		Matrix4 m(0,0,0, 10,20,30);
		Vec3 v1(4, 3, 2);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(14,23,32));
	}
	{
		Matrix4 m1(0,0,0, 10,20,30);
		Matrix4 m2(0,0,0, 3,4,5);
		Vec3 v1(0, 0, 0);
		Vec3 v2 = m1*m2*v1;
		assertEqual(v2, Vec3(13,24,35));
	}

	// Rotate around X axis.
	{
		Matrix4 m(180.0f, 0, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(2, 0, 0));
	}

	// Rotate around Y axis.
	{
		Matrix4 m(0, 180.0f, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(-2, 0, 0));
	}
	{
		Matrix4 m(0, 180.0f/2, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(0, 0, -2));
	}
	{
		Matrix4 m(0, -180.0f/2, 0, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(0, 0, 2));
	}

	// Rotate around Z axis.
	{
		Matrix4 m(0, 0, 180.0f, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(-2, 0, 0));
	}
	{
		Matrix4 m(0, 0, 180.0f/2, 0,0,0);
		Vec3 v1(2, 0, 0);
		Vec3 v2 = m*v1;
		assertEqual(v2, Vec3(0, 2, 0));
	}
	{
		Matrix4 m(0, 0, -180.0f/2, 0,0,0);
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


