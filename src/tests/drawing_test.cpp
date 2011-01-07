#include "graphics/opengl.h"
#include "graphics/window.h"
#include "userio.h"

#include "graphics_tests.h"

int main()
{
	size_t x = 600;
	size_t y = 400;

	Window w;
	w.createWindow(x, y);

	UserIO io;
	io.init();

	OpenGL opengl;

	while(io.getSingleKey() != "escape")
	{
		setup_camera(x, y);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw_box(0.2f);

		w.swap_buffers();
	}
}

