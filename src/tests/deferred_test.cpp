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
	w.createWindow(x * 3, y * 2);

	UserIO io;
	io.init();

	OpenGL opengl;

	Framebuffer deferredFBO = Framebuffer("deferredFBO", x, y, true, 2);
	deferredFBO.add_float_target();

	Shader shader("deferred.vertex", "deferred.fragment");

	Shader shader2("deferred2.vertex", "deferred2.fragment");
	shader2.start();
	shader2.set_texture_unit(0, "linearDepthTexture");
	shader2.set_texture_unit(1, "depthTexture");
	shader2.set_texture_unit(2, "realPosition");
	shader2.stop();

	while(io.getSingleKey() != "escape")
	{
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		setup_camera(x, y);

		deferredFBO.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.start();
		draw_box(0.2f);
		shader.stop();

		Framebuffer::unbind();

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		{
			// Actual reference image.
			glViewport(0, y, x, y);
			TextureHandler::getSingleton().bindTexture(0, deferredFBO.texture(0));
			TextureHandler::getSingleton().bindTexture(1, "");
			TextureHandler::getSingleton().bindTexture(2, "");
			drawFullscreenQuad();
			
			// Real reference positions.
			glViewport(0, 0, x, y);
			TextureHandler::getSingleton().bindTexture(0, deferredFBO.texture(2));
			TextureHandler::getSingleton().bindTexture(1, "");
			TextureHandler::getSingleton().bindTexture(2, "");
			drawFullscreenQuad();
		}

		{
			// Linear depth.
			glViewport(x, y, x, y);
			TextureHandler::getSingleton().bindTexture(0, deferredFBO.texture(1));
			TextureHandler::getSingleton().bindTexture(1, "");
			TextureHandler::getSingleton().bindTexture(2, "");
			drawFullscreenQuad();

			// Reconstruction with linear depth.
			glViewport(x, 0, x, y);
			TextureHandler::getSingleton().bindTexture(0, deferredFBO.texture(1));
			TextureHandler::getSingleton().bindTexture(1, deferredFBO.depth_texture());
			TextureHandler::getSingleton().bindTexture(2, deferredFBO.texture(2));
			shader2.start();
			glUniform1f(shader2.uniform("method"), 1.0f);
			drawFullscreenQuad();
			shader2.stop();
		}

		{
			// Default depth.
			glViewport(2*x, y, x, y);
			TextureHandler::getSingleton().bindTexture(0, deferredFBO.depth_texture());
			TextureHandler::getSingleton().bindTexture(1, "");
			TextureHandler::getSingleton().bindTexture(2, "");
			drawFullscreenQuad();

			setup_camera(x, y);

			// Reconstruction with default depth.
			glViewport(2*x, 0, x, y);
			TextureHandler::getSingleton().bindTexture(0, deferredFBO.texture(1));
			TextureHandler::getSingleton().bindTexture(1, deferredFBO.depth_texture());
			TextureHandler::getSingleton().bindTexture(2, deferredFBO.texture(2));
			shader2.start();
			glUniform1f(shader2.uniform("method"), 0.0f);
			drawFullscreenQuad();
			shader2.stop();
		}

		TextureHandler::getSingleton().bindTexture(0, "");
		TextureHandler::getSingleton().bindTexture(1, "");
		TextureHandler::getSingleton().bindTexture(2, "");

		w.swap_buffers();
	}
}

