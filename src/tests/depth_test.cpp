#include "graphics/opengl.h"
#include "graphics/window.h"
#include "userio.h"
#include "graphics/shader.h"
#include "graphics/framebuffer.h"
#include "graphics/texturehandler.h"
#include "graphics/graphics_helper.h"
#include "graphics/basiccamera.h"

#include "graphics_tests.h"

void setup_camera2(size_t x, size_t y)
{
	BasicCamera c;
	c.fov = 100.0f;
	c.aspect_ratio = float(x)/y;
	c.nearP = 0.3f;
	c.farP = 2.0f;
	Matrix4 p = c.perspective();

	c.setPosition(Vec3(0.4f,0.5f,0.3f));
	c.setTarget(Vec3(0.0f,0.0f,0.0f));
	Matrix4 m = c.modelview();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluPerspective(100.0f, float(x)/y, 0.3f, 0.8f);
	glMultTransposeMatrixf(p.T);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultTransposeMatrixf(m.T);
//	gluLookAt(0.4,0.3,0.5, 0,0,0, 0,1,0);
}

int main()
{
	size_t x = 600;
	size_t y = 400;

	Window w;
	w.createWindow(x, y);

	UserIO io;
	io.init();

	OpenGL opengl;

	Framebuffer screenFBO = Framebuffer("FBO", x, y, true, 1);
	Framebuffer screenFBO2 = Framebuffer("FBO2", x, y, true, 1);

	Shader shader("test2.vertex", "test2.fragment");

	while(io.getSingleKey() != "escape")
	{
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		setup_camera2(x, y);

		screenFBO2.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		screenFBO.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.start();
		draw_box(0.05f, 0.2f, 0.2f, 0.1f);
		draw_box(0.05f, -0.2f, 0.17f, 0.2f);
		shader.stop();

		screenFBO.bind_for_reading(); // TODO: doesn't work for depth buffer. Use shared (common) texture as depth buffer?
		screenFBO2.bind_for_writing();
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

		TextureHandler::getSingleton().bindTexture(0, screenFBO2.texture(0));
		drawFullscreenQuad();

		w.swap_buffers();
	}
}

