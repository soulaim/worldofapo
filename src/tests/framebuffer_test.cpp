#include "graphics/opengl.h"
#include "graphics/window.h"
#include "userio.h"
#include "graphics/shader.h"
#include "graphics/framebuffer.h"
#include "graphics/texturehandler.h"
#include "graphics/graphics_helper.h"

#include "graphics_tests.h"

int main()
{
	size_t x = 600;
	size_t y = 400;

	Window w;
	w.createWindow(x * 2, y);

	UserIO io;
	io.init();

	OpenGL opengl;

	Framebuffer screenFBO = Framebuffer("screenFBO", x, y, true, 1);

	Shader shader("test.vertex", "test.fragment");

	while(io.getSingleKey() != "escape")
	{
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		setup_camera(x, y);

		screenFBO.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.start();
		draw_box(0.2f, 0.0f, 0.0f, 0.0f);
		shader.stop();

		Framebuffer::unbind();

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glViewport(0, 0, x, y);
		TextureHandler::getSingleton().bindTexture(0, screenFBO.texture(0));
		drawFullscreenQuad();

		glViewport(x, 0, x, y);
		TextureHandler::getSingleton().bindTexture(0, screenFBO.depth_texture());
		drawFullscreenQuad();

		w.swap_buffers();
	}
}

