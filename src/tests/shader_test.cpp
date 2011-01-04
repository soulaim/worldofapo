#include "graphics/opengl.h"
#include "graphics/window.h"
#include "userio.h"
#include "graphics/shader.h"

void draw_box(float scale)
{
	GLfloat vertices[8 * 3] =
	{
		-scale, -scale, -scale,
		 scale, -scale, -scale,
		 scale,  scale, -scale,
		-scale,  scale, -scale,
		-scale,  scale,  scale,
		-scale, -scale,  scale,
		 scale, -scale,  scale,
		 scale,  scale,  scale
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

int main()
{
	Window w;
	w.createWindow(600, 400);

	UserIO io;
	io.init();

	OpenGL opengl;

	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.4,0.3,0.5, 0,0,0, 0,1,0);

	Shader shader("test.vertex", "test.fragment");
	while(io.getSingleKey() != "escape")
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.start();
		draw_box(0.2f);
		shader.stop();

		w.swap_buffers();
	}
}

