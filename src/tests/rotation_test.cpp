#include "graphics/frustum/vec3.h"
#include "algorithms.h"

#include "test.h"

void test_X_angles()
{
	{
		assertEqual(getXangle(Vec3( 0, 0, -1)),   0.0f);
		assertEqual(getXangle(Vec3(-1, 0,  0)),  90.0f);
		assertEqual(getXangle(Vec3( 0, 0,  1)), 180.0f);
		assertEqual(getXangle(Vec3( 1, 0,  0)), 270.0f);
	}

	{
		assertEqual(getYangle(Vec3( 0, 0, -1)), 90.0f);
		assertEqual(getYangle(Vec3(-1, 0,  0)), 90.0f);
		assertEqual(getYangle(Vec3( 0, 0,  1)), 90.0f);
		assertEqual(getYangle(Vec3( 1, 0,  0)), 90.0f);

	}

	{
		assertEqual(getXangle(Vec3( 0, 2, -1)),   0.0f);
		assertEqual(getXangle(Vec3(-1, 1,  0)),  90.0f);
		assertEqual(getXangle(Vec3( 0, 4,  1)), 180.0f);
		assertEqual(getXangle(Vec3( 1, 3,  0)), 270.0f);
	}

	{
		assertEqual(getXangle(Vec3(-1, 0, -1)),  45.0f);
		assertEqual(getXangle(Vec3(-1,-1,  1)), 135.0f);
		assertEqual(getXangle(Vec3( 1, 2,  1)), 225.0f);
		assertEqual(getXangle(Vec3( 1,-3, -1)), 315.0f);
	}
}

void test_Y_angles()
{
	{
		assertEqual(getYangle(Vec3( 0, 1, 0)), 180.0f);
		assertEqual(getYangle(Vec3( 0,-1, 0)),   0.0f);
	}

	{
		assertEqual(getYangle(Vec3(-1, 0,  0)), 90.0f);
		assertEqual(getYangle(Vec3( 0, 0, -2)), 90.0f);
		assertEqual(getYangle(Vec3( 3, 0,  0)), 90.0f);
		assertEqual(getYangle(Vec3( 0, 0,  4)), 90.0f);
	}
}

int main()
{
	test_X_angles();
	test_Y_angles();
}

