#ifndef PRIMITIVES_H
#define PRIMITIVES_H

struct ObjectTri
{
	float x[3];
	float y[3];
	float z[3];
};
/*
struct ObjectQuad
{
	float x[4];
	float y[4];
	float z[4];
};
*/
struct ObjectPart
{
//	std::vector<ObjectQuad> quads;
	std::vector<ObjectTri> triangles;
	
	float end_x;
	float end_y;
	float end_z;

	ObjectPart():
		end_x(0.0f),
		end_y(0.0f),
		end_z(0.0f)
	{
	}
};

#endif

