#include "graphics/opengl.h"
#include "graphics/window.h"
#include "userio.h"
#include "graphics/shader.h"

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

	Shader shader("test.vertex", "test.fragment");
	while(io.getSingleKey() != "escape")
	{
		setup_camera(x, y);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.start();
		draw_box(0.2f, 0.0f, 0.0f, 0.0f);
		shader.stop();

		w.swap_buffers();
	}
}

