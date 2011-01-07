#include "opengl.h"

void drawFullscreenQuad()
{
	float vertices[4*3] =
	{
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f
	};
	float texcoords[4*2] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
/*
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.0f, +1.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +1.0f, -1.0f);
	glEnd();
*/
}

