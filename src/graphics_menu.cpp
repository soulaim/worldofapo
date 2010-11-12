
#include "graphics.h"
#include "menubutton.h"
#include "texturehandler.h"

using namespace std;



void Graphics::drawMenu(const vector<MenuButton>& buttons) const
{
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	
	for(size_t i = 0; i < buttons.size(); ++i)
	{
		if(buttons[i].selected == 1)
			glColor3f(1.0f, 1.0f, 1.0f);
		else
			glColor3f(0.5f, 0.5f, 0.5f);
		
		float minus = 2.f * (i+0.f) / buttons.size() - 1.f;
		float plus  = 2.f * (i+1.f) / buttons.size() - 1.f;
		
		TextureHandler::getSingleton().bindTexture(0, buttons[i].name);
		
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.0f); glVertex3f(-1, minus, -1);
		glTexCoord2f(1.f, 0.0f); glVertex3f(1, minus, -1);
		glTexCoord2f(1.f, 1.0f); glVertex3f(1, plus, -1);
		glTexCoord2f(0.f, 1.0f); glVertex3f(-1, plus, -1);
		glEnd();
	}
	
	glDisable(GL_TEXTURE_2D);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	SDL_GL_SwapBuffers();
	
	if(lightsActive)
		glEnable(GL_LIGHTING);
	
	return;
}
