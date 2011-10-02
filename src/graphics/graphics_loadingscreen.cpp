
#include "graphics/graphics.h"
#include "graphics/texturehandler.h"
#include "graphics/shaders.h"
#include "graphics/hud.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/texturecoordinate.h"
#include "graphics/window.h"
#include "graphics/menus/menubutton.h"

#include <string>

void GameView::drawLoadScreen(const std::string& message, const std::string& bg_image, const float& percent_done)
{
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	float y_plus  = -0.85f;
	float y_minus = -0.95f;
	float plus  = (percent_done - 0.5f) * 2.0f;
	float minus = -1.0f;

	Graphics::Framebuffer::unbind();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// DRAW BACKGROUND IMAGE
	TextureHandler::getSingleton().bindTexture(0, bg_image);

	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	glTexCoord2f( 0.0f,  0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f( 1.0f,  0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	glTexCoord2f( 1.0f,  1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	glTexCoord2f( 0.0f,  1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();

	// DRAW LOAD BAR
	TextureHandler::getSingleton().unbindTexture(0);

	glColor3f(1.0f - percent_done, percent_done, 0.0f);

	glBegin(GL_QUADS);
	glVertex3f(minus, y_minus, -1.0f);
	glVertex3f( plus, y_minus, -1.0f);
	glVertex3f( plus,  y_plus, -1.0f);
	glVertex3f(minus,  y_plus, -1.0f);
	glEnd();

	// DRAW LOAD TITLE
	hud.drawString(message, -0.95f, -0.8f, 3.0f, true, 1.0f);

	window.swap_buffers();
	return;
}
