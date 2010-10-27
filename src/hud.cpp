#include "hud.h"
#include "graphics.h"
#include "texturehandler.h"

#include <string>
#include <sstream>
#include <map>
#include <iomanip>

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;
extern int QUADS_DRAWN_THIS_FRAME;

Hud::Hud():
	zombieCount(0),
	currentTime(0),
	world_ticks(0)
{
	charWidth.resize(255, 1.f);
	
	for(char symbol = 'A'; symbol <= 'Z'; symbol++)
		charWidth[symbol] = 0.22;
	for(char symbol = 'a'; symbol <= 'z'; symbol++)
		charWidth[symbol] = 0.19;
	for(char symbol = '0'; symbol <= '9'; symbol++)
		charWidth[symbol] = 0.16;
	
	charWidth['9'] = 0.20;
	charWidth['8'] = 0.20;
	charWidth['4'] = 0.20;
	charWidth['0'] = 0.23;
	charWidth['l'] = 0.1;
	charWidth['r'] = 0.1;
	charWidth['f'] = 0.1;
	charWidth['!'] = 0.1;
	charWidth['t'] = 0.15;
	charWidth['>'] = 0.15;
	charWidth['<'] = 0.15;
	charWidth['i'] = 0.1;
	charWidth['w'] = 0.25;
	charWidth['m'] = 0.25;
	charWidth['j'] = 0.12;
	charWidth['o'] = 0.19;
	charWidth['s'] = 0.12;
	charWidth['I'] = 0.1;
	charWidth['J'] = 0.12;
	charWidth['.'] = 0.1;
	charWidth[','] = 0.1;
	charWidth[':'] = 0.1;
	charWidth['?'] = 0.15;
	charWidth[' '] = 0.1;
	charWidth[']'] = 0.1;
	charWidth['['] = 0.1;
	charWidth[')'] = 0.1;
	charWidth['('] = 0.1;
	charWidth['\''] = 0.1;
	charWidth['-'] = 0.1;
	charWidth['|'] = 0.1;
	charWidth['/'] = 0.1;
	charWidth['_'] = 0.1;
	charWidth['%'] = 0.13;
	charWidth['&'] = 0.12;
	charWidth['='] = 0.13;
	charWidth['"'] = 0.1;
	
	charWidth['S'] = 0.17;
	charWidth['T'] = 0.19;
	charWidth['W'] = 0.24;
	charWidth['Q'] = 0.24;
	charWidth['O'] = 0.3;
	charWidth['Z'] = 0.1;
	charWidth['M'] = 0.3;
}

void Hud::setLocalPlayerName(const std::string& name)
{
	plr_name = name;
}

void Hud::setLocalPlayerHP(const int life)
{
	stringstream ss;
	ss << life;
	health = ss.str();
}

void Hud::setLocalPlayerKills(const int k)
{
	stringstream ss;
	ss << k;
	kills = ss.str();
}

void Hud::setLocalPlayerDeaths(const int d)
{
	stringstream ss;
	ss << d;
	deaths = ss.str();
}

void Hud::pushMessage(const string& msg)
{
	viewMessages.push_back(ViewMessage(msg, currentTime));
}

void Hud::setTime(unsigned time)
{
	currentTime = time;
}

void Hud::setCurrentClientCommand(const string& cmd)
{
	currentClientCommand = cmd;
}

void Hud::setMinimapHumanPositions(const std::vector<Location>& positions)
{
	humanPositions = positions;
}

void Hud::world_tick()
{
	++world_ticks;
}

void Hud::setPlayerInfo(map<int,PlayerInfo> *pInfo)
{
	Players = pInfo;
}

void Hud::setZombiesLeft(int count)
{
	zombieCount = count;
}

void Hud::setMinimap(float angle, const Location& unit)
{
	minimap_angle = angle;
	unit_location = unit;
}


void Hud::drawStatusBar() const
{
	stringstream ss;
	ss << plr_name << " hp: " << health << " k/d: " << kills << "/" << deaths;
	drawString(ss.str(), -0.9, 0.9, 2.0, true);
}

void Hud::drawBanner() const
{
	drawString("World of Apo");
}

void Hud::drawZombiesLeft() const
{
	stringstream zz;
	zz << "Zombies left: " << zombieCount;
	drawString(zz.str(), 0.2, 0.9, 1.5, true);
}


void Hud::drawMessages()
{
	for(size_t i = 0; i < viewMessages.size(); ++i)
	{
		if(viewMessages[i].endTime < currentTime)
		{
			for(size_t k = i+1; k < viewMessages.size(); ++k)
				viewMessages[k-1] = viewMessages[k];
			viewMessages.pop_back();
			
			i--;
			continue;
		}
		
		float pos_x = -0.9;
		float pos_y = -0.82 + 0.08 * ( viewMessages.size() - (i+1) );
		drawString(viewMessages[i].msgContent, pos_x, pos_y, viewMessages[i].scale, viewMessages[i].hilight);
	}
	
	if(currentClientCommand.size() > 0)
		drawString(currentClientCommand, -0.9, -0.9, 1.3, true);
}

void Hud::drawFPS()
{
	static int last_time = SDL_GetTicks();
	static double fps = 0;
	static double world_fps = 0;
	static int frames = 0;
	++frames;
	int time_now = SDL_GetTicks();
	int time_since_last = time_now - last_time;
	if(time_since_last > 500)
	{
		fps = 1000.0 * frames / time_since_last;
		world_fps = 1000.0 * world_ticks/ time_since_last;
		frames = 0;
		world_ticks = 0;
		last_time = time_now;
	}


	stringstream ss1;
	ss1 << "FPS: " << fixed << setprecision(2) << fps;
	stringstream ss2;
	ss2 << "TPS: " << fixed << setprecision(2) << world_fps;
	stringstream ss3;
	ss3 << "TRIS: " << fixed << setprecision(2) << TRIANGLES_DRAWN_THIS_FRAME;
	stringstream ss4;
	ss4 << "QUADS: " << fixed << setprecision(2) << QUADS_DRAWN_THIS_FRAME;
	drawString(ss1.str(), 0.6, 0.9, 1.5, true);
	drawString(ss2.str(), 0.6, 0.8, 1.5, true);
	drawString(ss3.str(), 0.6, 0.7, 1.5, true);
	drawString(ss4.str(), 0.6, 0.6, 1.5, true);
}

void Hud::draw(bool firstPerson)
{
	if(firstPerson)
	{
		drawCrossHair();
	}

	drawMessages();
	drawStatusBar();
	drawMinimap();
	drawZombiesLeft();
	drawBanner();
	drawStats();
	drawFPS();
}


void Hud::drawStats() const
{
	int i = 0;
	for(map<int, PlayerInfo>::iterator iter = Players->begin(); iter != Players->end(); iter++)
	{
		if (iter->first < 0)
		  continue;
		stringstream line;
		line << iter->second.name << " " << iter->second.kills << "/" << iter->second.deaths;
		drawString(line.str(), -0.8f, 0.8f-(i*0.05), 1.0f);
		i++;
	}
}

void Hud::drawCrossHair() const
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	TextureHandler::getSingleton().bindTexture("crosshair");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-0.03f, +0.02f, -1);
	glTexCoord2f(1.f, 0.f); glVertex3f(+0.03f, +0.02f, -1);
	glTexCoord2f(1.f, 1.f); glVertex3f(+0.03f, +0.08f, -1);
	glTexCoord2f(0.f, 1.f); glVertex3f(-0.03f, +0.08f, -1);
	glEnd();
	
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
//	if(lightsActive)
//		glEnable(GL_LIGHTING);
}

void Hud::drawString(const string& msg, float pos_x, float pos_y, float scale, bool background) const
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	TextureHandler::getSingleton().bindTexture("font");
	
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
		
		totalWidth += 0.05 * charWidth[msg[i]] * 2 * scale;
	}
	
	float x_now     = 0.0f;
	float x_next    = pos_x + 0.05;
	float y_bot     = pos_y;
	float y_top     = pos_y + 0.05 * scale;
	float edge_size = 1./16.;
	
	// draw a darker background box for the text if that was requested
	if(background)
	{
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
		glBegin(GL_QUADS);
		glVertex3f(pos_x - 0.01 * scale , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_top, -1);
		glVertex3f(pos_x - 0.01 * scale , y_top, -1);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	
	float currentWidth = 0.f;
	float lastWidth    = 0.f;
	
	// reset default colour to white.
	glColor4f(1.0f, 1.0f, 1.0f, 1.f);
	
	glBegin(GL_QUADS);
	for(size_t i = 0; i < msg.size(); ++i)
	{
		if(msg[i] == '^')
		{
			++i;
			if(msg[i] == 'g')
				glColor4f(0.1f, 0.6f, 0.1f, 1.0f);
			else if(msg[i] == 'G')
				glColor4f(0.1f, 1.0f, 0.1f, 1.0f);
			else if(msg[i] == 'r')
				glColor4f(0.6f, 0.1f, 0.1f, 1.0f);
			else if(msg[i] == 'R')
				glColor4f(1.0f, 0.1f, 0.1f, 1.0f);
			else if(msg[i] == 'b')
				glColor4f(0.1f, 0.1f, 0.6f, 1.0f);
			else if(msg[i] == 'B')
				glColor4f(0.1f, 0.1f, 1.0f, 1.0f);
			else if(msg[i] == 'y')
				glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
			else if(msg[i] == 'Y')
				glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
			else if(msg[i] == 'W')
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			else if(msg[i] == 'w')
				glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
			
			continue;
		}
		
		currentWidth = 0.05 * charWidth[msg[i]];
		
		x_now = x_next + scale * (currentWidth + lastWidth - 0.05f);
		x_next = x_now + 0.05f * scale;
		
		lastWidth = currentWidth;
		
		int x = msg[i] % 16;
		int y = 15 - (msg[i] / 16);

		glTexCoord2f( x    * edge_size, (y+1) * edge_size); glVertex3f(x_now , y_top, -1);
		glTexCoord2f((x+1) * edge_size, (y+1) * edge_size); glVertex3f(x_next, y_top, -1);
		glTexCoord2f((x+1) * edge_size, y * edge_size);     glVertex3f(x_next, y_bot, -1);
		glTexCoord2f( x    * edge_size, y * edge_size);     glVertex3f(x_now , y_bot, -1);
	}
	glEnd();
	glDisable(GL_BLEND);
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void Hud::drawMinimap() const
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef(0.0f, 0.0f, -1.0f);

	glTranslatef(0.78f, -0.78f, 0.0f);
	glRotatef(minimap_angle, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.78f, 0.78f, 0.0f);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
	glBegin(GL_QUADS);
	glVertex3f(0.60f, -0.96f, 0.f);
	glVertex3f(0.96f, -0.96f, 0.f);
	glVertex3f(0.96f, -0.60f, 0.f);
	glVertex3f(0.60f, -0.60f, 0.f);
	glEnd();
	
	glPointSize(4.0f);

	glBegin(GL_POINTS);
	for(std::vector<Location>::const_iterator iter = humanPositions.begin(); iter != humanPositions.end(); ++iter)
	{
		const Location& loc = *iter;
		const Location& unitPos = unit_location;
		if(loc == unitPos)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else
		{
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		glVertex3f(0.96f - (0.37*loc.x.getFloat())/800.0f, -0.96f + (0.37*loc.z.getFloat())/800.0f, 0.f);
	}
	glEnd();
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

