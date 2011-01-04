#include "graphics/frustum/vec3.h"
#include "algorithms.h"

#include <iostream>
#include <cmath>
#include <array>

using namespace std;


int main()
{
	int width = 800;
	int height = 600;
	float x, y, z;

	while(cin >> x >> y >> z)
	{
		array<int,4> rect;
		int n = light_scissor(Vec3(x, y, z), 1, width, height, rect);
		cout << n << " / " << width*height << endl;
		cout << "[" << rect[0] << "," << rect[2] << "] x [" << rect[1] << "," << rect[3] << "]" << endl;
	}
}

