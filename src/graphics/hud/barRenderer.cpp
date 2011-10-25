
#include "graphics/hud/barRenderer.h"
#include "graphics/opengl.h"
#include "graphics/texturehandler.h"
#include "graphics/string2rgb.h"
#include "graphics/frustum/matrix4.h"

using std::string;

void BarRenderer::draw3DBar(float percentage, const vec3<float>& pos, float x_angle, float y_angle, const std::string& start_color, const std::string& end_color, float scale) const
{
	TextureHandler::getSingleton().unbindTexture(0);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Determine color

	float rgb1[4];
	float rgb2[4];
	float rgb_current[4];

	getColor(start_color, rgb1);
	getColor(end_color, rgb2);

	for(int i=0; i<3; i++)
		rgb_current[i] = rgb2[i] + (rgb1[i] - rgb2[i]) * percentage;
	rgb_current[3] = 0.6f;

	glColor4fv(rgb_current);

	// Build transformation matrix
	y_angle = -y_angle + 90.f;
	Matrix4 m(y_angle, x_angle, 0, pos.x, pos.y, pos.z);

	float x_val = scale * percentage;

	glBegin(GL_QUADS);
	{
		vec3<float> p1 = m * vec3<float>(-x_val, +scale * 0.1f, 0);
		vec3<float> p2 = m * vec3<float>(-x_val, -scale * 0.1f, 0);
		vec3<float> p3 = m * vec3<float>(+x_val, -scale * 0.1f, 0);
		vec3<float> p4 = m * vec3<float>(+x_val, +scale * 0.1f, 0);

		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);
		glVertex3f(p4.x, p4.y, p4.z);
	}
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}




void BarRenderer::drawBar(float size, const string& start_color, const string& end_color, float min_x, float max_x, float min_y, float max_y) const
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	TextureHandler::getSingleton().unbindTexture(0);

	float width = max_x - min_x;

	float rgb1[4];
	float rgb2[4];
	float rgb_current[4];

	getColor(start_color, rgb1);
	getColor(end_color, rgb2);

	for(int i=0; i<3; i++)
		rgb_current[i] = rgb2[i] + (rgb1[i] - rgb2[i]) * size;
	rgb_current[3] = 1.0f;

	glColor4fv(rgb_current);

	glBegin(GL_QUADS);
		glVertex3f(min_x, min_y, 1.0f);
		glVertex3f(min_x + width * size, min_y, 1.0f);
		glVertex3f(min_x + width * size, max_y, 1.0f);
		glVertex3f(min_x, max_y, 1.0f);
	glEnd();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
