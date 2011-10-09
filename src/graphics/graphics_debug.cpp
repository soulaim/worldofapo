
#include "graphics/graphics.h"
#include "graphics/texturehandler.h"
#include "world/level.h"
#include "graphics/shaders.h"
#include "graphics/hud/hud.h"
#include "graphics/frustum/matrix4.h"
#include "physics/octree.h"
#include "graphics/texturecoordinate.h"
#include "graphics/window.h"
#include "graphics/menus/menubutton.h"
#include "algorithms.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>
#include <numeric>
#include <map>
#include <stdexcept>
#include <list>
#include <tuple>

using namespace std;

vector<pair<vec3<float>, vec3<float> > > LINES;
vector<vec3<float> > DOTS;
vector<pair<vec3<float>, string> > STRINGS;

extern int TRIANGLES_DRAWN_THIS_FRAME;
extern int QUADS_DRAWN_THIS_FRAME;


void GameView::toggleWireframeStatus()
{
	int wireframe = intVals["DRAW_DEBUG_WIREFRAME"];
	int level = intVals["DRAW_LEVEL"];
	intVals["DRAW_DEBUG_WIREFRAME"] = 1 - wireframe;
	intVals["DRAW_LEVEL"] = 1 - level;
}

void GameView::toggleDebugStatus()
{
	int old = intVals["DRAW_DEBUG_LINES"];
	intVals["DRAW_DEBUG_LINES"] = 1 - old;
}


void GameView::drawDebugLines()
{
	glLineWidth(2.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	for(size_t i = 0; i < LINES.size(); ++i)
	{
		const vec3<float>& p1 = LINES[i].first;
		const vec3<float>& p2 = LINES[i].second;
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
	}
	glEnd();

	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for(size_t i = 0; i < DOTS.size(); ++i)
	{
		const vec3<float>& p1 = DOTS[i];
		glVertex3f(p1.x, p1.y, p1.z);
	}
	glEnd();
}

void GameView::drawDebugStrings()
{
	for(size_t i = 0; i < STRINGS.size(); ++i)
	{
		textRenderer.draw3Dstring(STRINGS[i].second, STRINGS[i].first, camera_p->getXrot(), camera_p->getYrot());
	}
}


// This function is not in use anymore, would be useful only if geometry shader is not available. Remove if you feel like it.
void GameView::drawParticles_old(std::vector<Particle>& viewParticles, const std::string& depth_texture)
{
    prepareForParticleRendering(depth_texture);
	Graphics::Framebuffer& particlesUpScaledFBO = Graphics::Framebuffer::get("particlesUpScaledFBO");

	float x_angle =  camera_p->getXrot();
	float y_angle = -camera_p->getYrot() - 90.f;
	Matrix4 m(y_angle, x_angle, 0.f, 0.f, 0.f, 0.f);

	vec3<float> s1(-1.0f, -1.0f, 0.0f);
	vec3<float> s2(+1.0f, -1.0f, 0.0f);
	vec3<float> s3(+1.0f, +1.0f, 0.0f);
	vec3<float> s4(-1.0f, +1.0f, 0.0f);

	s1 = m * s1;
	s2 = m * s2;
	s3 = m * s3;
	s4 = m * s4;

	float color[4];

    glBegin(GL_QUADS);

	for(size_t i = 0; i < viewParticles.size(); ++i)
	{

		float px = viewParticles[i].pos.x;
		float py = viewParticles[i].pos.y;
		float pz = viewParticles[i].pos.z;

		viewParticles[i].getColor(color);
		color[3] = viewParticles[i].getAlpha();
		glColor4fv(color);

		float s = viewParticles[i].getScale();
		glTexCoord2f(0.f, 0.f); glVertex3f(px+s1.x*s, py+s1.y*s, pz+s1.z*s);
		glTexCoord2f(1.f, 0.f); glVertex3f(px+s2.x*s, py+s2.y*s, pz+s2.z*s);
		glTexCoord2f(1.f, 1.f); glVertex3f(px+s3.x*s, py+s3.y*s, pz+s3.z*s);
		glTexCoord2f(0.f, 1.f); glVertex3f(px+s4.x*s, py+s4.y*s, pz+s4.z*s);

		++QUADS_DRAWN_THIS_FRAME;
	}

	glEnd();

    // upscale particle rendering result
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_BLEND);
	Graphics::Framebuffer::get("particlesUpScaledFBO").bind();

	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("particlesDownScaledFBO").texture(0));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// TODO: Could this perhaps be done (faster?) by directly copying from one framebuffer to another?
	// Perhaps with with glBindGraphics::Framebuffer(GL_READ_FRAMEBUFFER,...); glBindGraphics::Framebuffer(GL_DRAW_FRAMEBUFFER,...); glBlitGraphics::Framebuffer(...);??
	drawFullscreenQuad(); // Draw particlesDownScaledFBO_texture to particlesUpScaledFBO_texture (i.e. upscale).

	// blur upscaled particle texture
	if(intVals["PARTICLE_BLUR"])
	{
		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], particlesUpScaledFBO.texture(0), particlesUpScaledFBO);

		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], particlesUpScaledFBO.texture(0), particlesUpScaledFBO);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Graphics::Framebuffer::get("screenFBO").bind();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	TextureHandler::getSingleton().bindTexture(0, particlesUpScaledFBO.texture(0));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	drawFullscreenQuad(); // Draw particlesUpScaledFBO_texture to screenFBO_texture (copy directly).

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	TextureHandler::getSingleton().unbindTexture(0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}


void GameView::drawDebugQuad()
{
	string s = strVals["DEBUG_QUAD"];
	if(s == "NULL")
	{
		return;
	}

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	TextureHandler::getSingleton().bindTexture(0, s);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -0.7f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(-0.2f, -0.7f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(-0.2f, +0.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +0.0f, -1.0f);
	glEnd();

	TextureHandler::getSingleton().unbindTexture(0);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}


void GameView::drawBox(const Location& top, const Location& bot, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	glColor4f(r, g, b, a);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());

	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());

	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glEnd();
}

void GameView::drawOctree(const std::shared_ptr<Octree>& o)
{
	if (o->n > 0)
		drawBox(o->top, o->bot, 1.0f);
	if (o->hasChildren) {
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
					drawOctree(o->children[i][j][k]);
	}
}


void GameView::drawBoundingBoxes(const std::map<int,WorldItem>& items)
{
	for(auto iter = items.begin(); iter != items.end(); iter++)
	{
		drawBox(iter->second.bb_top(), iter->second.bb_bot());
	}
}

void GameView::drawBoundingBoxes(const std::map<int,Unit>& units)
{
	for(auto iter = units.begin(); iter != units.end(); iter++)
	{
		drawBox(iter->second.bb_top(), iter->second.bb_bot());
	}
}

void GameView::drawDebugProjectiles(const std::map<int, Projectile>& projectiles)
{
	glColor3f(0.3, 0.7, 0.5);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for(auto it = projectiles.begin(); it != projectiles.end(); ++it)
	{
		const MovableObject& proj = it->second;
		glVertex3f(proj.position.x.getFloat(), proj.position.y.getFloat(), proj.position.z.getFloat());
	}
	glEnd();
}










void GameView::drawMenuRectangles() const
{
	static list<tuple<float, float, float, float, float, float, float> > rects;
	static int count = 0;
	if(count == 0)
	{
		rects.push_back(make_tuple(0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.4f, 0.4f));
	}
	if(count++ % 30 == 0)
	{
		float r = sin(count / 300.0f) * 0.5 + 0.5;
		float g = cos(count / 300.0f) * 0.5 + 0.5;
		float b = (sin(count / 300.0f) + cos(count / 300.0f))/2 * 0.5 + 0.5;

		float x = sin(count / 300.0f) / 5.0f;
		float y = cos(count / 300.0f) / 5.0f;
		rects.push_back(make_tuple(x, y, 0.007f, 0.004f, r, g, b));
	}
	TextureHandler::getSingleton().unbindTexture(0);

	for(auto it = rects.begin(); it != rects.end(); )
	{
		float center_x = std::get<0>(*it);
		float center_y = std::get<1>(*it);
		float size_x = std::get<2>(*it);
		float size_y = std::get<3>(*it);
		float r = std::get<4>(*it);
		float g = std::get<5>(*it);
		float b = std::get<6>(*it);

		float left = center_x - size_x;
		float right = center_x + size_x;
		float top = center_y + size_y;
		float bot = center_y - size_y;

		if(top > 1.0f && bot < -1.0f && left < -1.0f && right > 1.0f)
		{
			it = rects.erase(it);
			continue;
		}
		else
		{
			std::get<2>(*it) *= 1.03f;
			std::get<3>(*it) *= 1.03f;
			++it;
		}

		glLineWidth(3.0f);
		glColor3f(r, g, b);
		glBegin(GL_LINES);
		glVertex3f(left, top, -1.0f);
		glVertex3f(right, top, -1.0f);
		glVertex3f(left, bot, -1.0f);
		glVertex3f(right, bot, -1.0f);
		glVertex3f(left, top, -1.0f);
		glVertex3f(left, bot, -1.0f);
		glVertex3f(right, top, -1.0f);
		glVertex3f(right, bot, -1.0f);
		glEnd();
	}
}