#ifndef GRAPHICS_TESTS_H
#define GRAPHICS_TESTS_H

#include "graphics/opengl.h"

void setup_camera(size_t x, size_t y)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(100.0f, float(x)/y, 0.3f, 0.8f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.4,0.3,0.5, 0,0,0, 0,1,0);
}

void draw_box(float scale, float center_x, float center_y, float center_z)
{
	GLfloat vertices[8 * 3] =
	{
		-scale + center_x, -scale + center_y, -scale + center_z,
		 scale + center_x, -scale + center_y, -scale + center_z,
		 scale + center_x,  scale + center_y, -scale + center_z,
		-scale + center_x,  scale + center_y, -scale + center_z,
		-scale + center_x,  scale + center_y,  scale + center_z,
		-scale + center_x, -scale + center_y,  scale + center_z,
		 scale + center_x, -scale + center_y,  scale + center_z,
		 scale + center_x,  scale + center_y,  scale + center_z
	};
	GLfloat colors[8 * 3] =
	{
		1, 1, 1,
		1, 1, 0,
		1, 0, 1,
		1, 0, 0,
		0, 1, 1,
		0, 1, 0,
		0, 0, 1,
		0, 0, 0
	};
	GLubyte indices[6 * 4] =
	{
		0,1,2,3,
		0,3,4,5,
		0,5,6,1,
		1,6,7,2,
		7,4,3,2,
		4,7,6,5
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, 0, colors);

	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

#endif

