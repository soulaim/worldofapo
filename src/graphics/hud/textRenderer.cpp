
#include "graphics/hud/textRenderer.h"
#include "graphics/opengl.h"
#include "graphics/texturehandler.h"
#include "graphics/font.h"
#include "graphics/frustum/matrix4.h"

#include <string>

using namespace std;

void choose_color(char code, float alpha)
{
	switch(code)
	{
		case 'g': glColor4f(0.1f, 0.6f, 0.1f, alpha); break;
		case 'G': glColor4f(0.1f, 1.0f, 0.1f, alpha); break;
		case 'r': glColor4f(0.6f, 0.1f, 0.1f, alpha); break;
		case 'R': glColor4f(1.0f, 0.1f, 0.1f, alpha); break;
		case 'b': glColor4f(0.1f, 0.1f, 0.6f, alpha); break;
		case 'B': glColor4f(0.1f, 0.1f, 1.0f, alpha); break;
		case 'y': glColor4f(0.7f, 0.7f, 0.0f, alpha); break;
		case 'Y': glColor4f(1.0f, 1.0f, 0.0f, alpha); break;
		case 'W': glColor4f(1.0f, 1.0f, 1.0f, alpha); break;
		case 'w': glColor4f(0.6f, 0.6f, 0.6f, alpha); break;
		default: break;
	}
}

void choose_team_color(int team)
{
	switch(team)
	{
		case 0: glColor3f(0.0f, 1.0f, 0.0f); break;
		case 1: glColor3f(1.0f, 0.0f, 0.0f); break;
		case 2: glColor3f(0.0f, 0.0f, 1.0f); break;
		case 3: glColor3f(1.0f, 0.0f, 1.0f); break;
		case 4: glColor3f(1.0f, 1.0f, 0.0f); break;
		case 5: glColor3f(0.0f, 1.0f, 1.0f); break;
		case 6: glColor3f(1.0f, 5.0f, 0.0f); break;
		case 7: glColor3f(0.0f, 1.0f, 5.0f); break;
		default: glColor3f(1.0f, 1.0f, 1.0f); break;
	};
}


int TextRenderer::draw3Dstring(const string& message, const vec3<float>& pos, float x_angle, float y_angle, int team) const
{
    int quadsDrawn = 0;
	string msg;

	if(team == 0)
	{
		msg = "^G" + message + "^W";
	}
	else if((team == 0) || (team == 1))
	{
		msg = "^R" + message + "^W";
	}
	else
	{
		msg = message;
	}

	float scale = 50.0f;

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	TextureHandler::getSingleton().bindTexture(0, "font");

	float totalWidth = 0.0f;
	float aux_width = -0.05f;
	float currentWidth = 0.f;
	float lastWidth    = 0.f;

	for(size_t i = 0; i < msg.size(); ++i)
	{
		if(msg[i] == '^')
		{
			++i;
			continue;
		}

		char current_symbol = 'Q';
		if(msg[i] == '\\')
		{
			++i;
			if(msg[i] == 's')
			{
				current_symbol = ' ';
			}
			else
			{
				cerr << "WARNING: Unrecognized escape sequence in draw3Dstring!" << endl;
			}
		}
		else
		{
			current_symbol = msg[i];
		}

		currentWidth = 0.05 * Font::width(current_symbol);
		totalWidth = aux_width - scale * (currentWidth + lastWidth - 0.05f);
		aux_width = totalWidth - 0.05f * scale;
		lastWidth = currentWidth;

		// totalWidth += 0.05 * Font::width(msg[i]) * 2 * scale;
	}

	float halfWidth = -totalWidth / 2.0f;

	float x_now     = halfWidth + 0.35;
	float x_next    = x_now - 0.05;
	float y_bot     = 0.0f;
	float y_top     = 0.05 * scale;
	currentWidth = 0.f;
	lastWidth    = 0.f;

	choose_team_color(team);

	y_angle = -y_angle + 90.f;
	Matrix4 m(y_angle, x_angle, 0, pos.x, pos.y, pos.z);

	glBegin(GL_QUADS);
	for(size_t i = 0; i < msg.size(); ++i)
	{
		char next_char;
		if(msg[i] == '^')
		{
			++i;
			choose_color(msg[i], 1.0f);

			continue;
		}
		else if(msg[i] == '\\')
		{
			if(msg[i+1] == 's') // space symbol
				next_char = ' ';
			else
				next_char = msg[i+1];
			++i;
		}
		else
		{
			next_char = msg[i];
		}

		currentWidth = 0.05 * Font::width(next_char);
		x_now = x_next - scale * (currentWidth + lastWidth - 0.05f);
		x_next = x_now - 0.05f * scale;

		lastWidth = currentWidth;

		vec3<float> p1 = m * vec3<float>(x_now , y_bot, 0);
		vec3<float> p2 = m * vec3<float>(x_next, y_bot, 0);
		vec3<float> p3 = m * vec3<float>(x_next, y_top, 0);
		vec3<float> p4 = m * vec3<float>(x_now , y_top, 0);

		TextureCoordinates coords = Font::texture_coordinates(next_char);
		glTexCoord2f(coords.corner[0].x, coords.corner[0].y); glVertex3f(p1.x, p1.y, p1.z);
		glTexCoord2f(coords.corner[1].x, coords.corner[1].y); glVertex3f(p2.x, p2.y, p2.z);
		glTexCoord2f(coords.corner[2].x, coords.corner[2].y); glVertex3f(p3.x, p3.y, p3.z);
		glTexCoord2f(coords.corner[3].x, coords.corner[3].y); glVertex3f(p4.x, p4.y, p4.z);

		++quadsDrawn;
	}
	glEnd();
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);

    return quadsDrawn;
}



int TextRenderer::drawString(const string& msg, float pos_x, float pos_y, float scale, bool background, float alpha) const
{
    int quadsDrawn = 0;

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	TextureHandler::getSingleton().bindTexture(0, "font");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float totalWidth = 0.025f;
	for(size_t i = 0; i < msg.size(); ++i)
	{
		if(msg[i] == '^')
		{
			++i;
			continue;
		}

		totalWidth += 0.05 * Font::width(msg[i]) * 2 * scale;
	}

	float x_now     = 0.0f;
	float x_next    = pos_x + 0.05;
	float y_bot     = pos_y;
	float y_top     = pos_y + 0.05 * scale;

	// draw a darker background box for the text if that was requested
	glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
	glBegin(GL_QUADS);
	if(background)
	{
		glVertex3f(pos_x - 0.01 * scale  , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_top, -1);
		glVertex3f(pos_x - 0.01 * scale  , y_top, -1);
		++quadsDrawn;
	}
	glEnd();

	float currentWidth = 0.f;
	float lastWidth    = 0.f;

	// reset default colour to white.
	glColor4f(1.0f, 1.0f, 1.0f, alpha);

	glBegin(GL_QUADS);
	for(size_t i = 0; i < msg.size(); ++i)
	{
		char next_char;
		if(msg[i] == '^')
		{
			++i;
			choose_color(msg[i], alpha);

			continue;
		}
		else if(msg[i] == '\\')
		{
			if(msg[i+1] == 's') // space symbol
				next_char = ' ';
			else
				next_char = msg[i+1];
			++i;
		}
		else
		{
			next_char = msg[i];
		}

		currentWidth = 0.05 * Font::width(next_char);

		x_now = x_next + scale * (currentWidth + lastWidth - 0.05f);
		x_next = x_now + 0.05f * scale;

		lastWidth = currentWidth;

		TextureCoordinates coords = Font::texture_coordinates(next_char);
		glTexCoord2f(coords.corner[0].x, coords.corner[0].y); glVertex3f(x_now , y_bot, -1);
		glTexCoord2f(coords.corner[1].x, coords.corner[1].y); glVertex3f(x_next, y_bot, -1);
		glTexCoord2f(coords.corner[2].x, coords.corner[2].y); glVertex3f(x_next, y_top, -1);
		glTexCoord2f(coords.corner[3].x, coords.corner[3].y); glVertex3f(x_now , y_top, -1);

		++quadsDrawn;
	}
	glEnd();
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

    return quadsDrawn;
}