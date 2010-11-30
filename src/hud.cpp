#include "hud.h"
#include "graphics.h"
#include "texturehandler.h"
#include "apomath.h"

#include <SDL.h>

#include <string>
#include <sstream>
#include <map>
#include <iomanip>
#include <cmath>

#include "frustum/matrix4.h"

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;
extern int QUADS_DRAWN_THIS_FRAME;

Hud::Hud():
	showStats(false),
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
	charWidth['+'] = 0.1;
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

void Hud::setLevelSize(int x, int z) {
	level_max_x = x;
	level_max_z = z;
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

void Hud::world_tick()
{
	++world_ticks;
}

void Hud::setUnitsMap(std::map<int, Unit>* _units)
{
	units = _units;
}

void Hud::setLocalPlayerID(int _myID)
{
	myID = _myID;
}

void Hud::setPlayerInfo(map<int,PlayerInfo> *pInfo)
{
	Players = pInfo;
}

void Hud::setZombiesLeft(int count)
{
	zombieCount = count;
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
		
		int reverse_index = viewMessages.size() - (i+1);
		float pos_x = -0.9;
		float pos_y = -0.82 + 0.05 * reverse_index;
		
		float location_alpha = 1.0f - reverse_index * 0.045f;
		float age_alpha      = float(viewMessages[i].endTime - currentTime) / ViewMessage::VIEW_MESSAGE_LIFE;
		float alpha          = age_alpha * location_alpha;
		
		if(alpha < 0.f)
			alpha = 0.f;
		drawString(viewMessages[i].msgContent, pos_x, pos_y, viewMessages[i].scale, viewMessages[i].hilight, alpha);
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
	drawString(ss1.str(), 0.6, 0.9, 1.5, true);
	stringstream ss2;
	ss2 << "TPS: " << fixed << setprecision(2) << world_fps;
	drawString(ss2.str(), 0.6, 0.8, 1.5, true);
	stringstream ss3;
	ss3 << "TRIS: " << fixed << setprecision(2) << TRIANGLES_DRAWN_THIS_FRAME;
	drawString(ss3.str(), 0.6, 0.7, 1.5, true);
	stringstream ss4;
	ss4 << "QUADS: " << fixed << setprecision(2) << QUADS_DRAWN_THIS_FRAME;
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

void Hud::setShowStats(bool _showStats) {
	showStats = _showStats;
}

void Hud::drawStats() const
{
	if (!showStats)
		return;

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
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	TextureHandler::getSingleton().bindTexture(0, "crosshair");
	
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-0.03f, -0.03f, -1);
	glTexCoord2f(1.f, 0.f); glVertex3f(+0.03f, -0.03f, -1);
	glTexCoord2f(1.f, 1.f); glVertex3f(+0.03f, +0.03f, -1);
	glTexCoord2f(0.f, 1.f); glVertex3f(-0.03f, +0.03f, -1);
	++QUADS_DRAWN_THIS_FRAME;
	glEnd();
	
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
//	if(lightsActive)
//		glEnable(GL_LIGHTING);
}

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

void Hud::draw3Dstring(const string& msg, const Vec3& pos, float x_angle, float y_angle, int team) const
{
	float scale = 50.0f;

	glDisable(GL_LIGHTING);
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
		
		currentWidth = 0.05 * charWidth[msg[i]];
		totalWidth = aux_width - scale * (currentWidth + lastWidth - 0.05f);
		aux_width = totalWidth - 0.05f * scale;
		lastWidth = currentWidth;
		
		// totalWidth += 0.05 * charWidth[msg[i]] * 2 * scale;
	}
	
	float halfWidth = -totalWidth / 2.0f;
	
	float x_now     = halfWidth + 0.35;
	float x_next    = x_now - 0.05;
	float y_bot     = 0.0f;
	float y_top     = 0.05 * scale;
	float edge_size = 1./16.;
	currentWidth = 0.f;
	lastWidth    = 0.f;
	
	switch(team)
	{
		case 0:
			glColor3f(0.0f, 1.0f, 0.0f);
			break;
		case 1:
			glColor3f(1.0f, 0.0f, 0.0f);
			break;
		case 2:
			glColor3f(0.0f, 0.0f, 1.0f);
			break;
		case 3:
			glColor3f(1.0f, 0.0f, 1.0f);
			break;
		case 4:
			glColor3f(1.0f, 1.0f, 0.0f);
			break;
		case 5:
			glColor3f(0.0f, 1.0f, 1.0f);
			break;
		case 6:
			glColor3f(1.0f, 5.0f, 0.0f);
			break;
		case 7:
			glColor3f(0.0f, 1.0f, 5.0f);
			break;
		default:
			glColor3f(1.0f, 1.0f, 1.0f);
			break;
	};
	
	y_angle = -y_angle + 90.f;
	Matrix4 m(y_angle, x_angle, 0, pos.x, pos.y, pos.z);
	
	glBegin(GL_QUADS);
	for(size_t i = 0; i < msg.size(); ++i)
	{
		if(msg[i] == '^')
		{
			++i;
			choose_color(msg[i], 1.0f);
			continue;
		}
		
		currentWidth = 0.05 * charWidth[msg[i]];
		x_now = x_next - scale * (currentWidth + lastWidth - 0.05f);
		x_next = x_now - 0.05f * scale;
		
		lastWidth = currentWidth;
		
		int x = msg[i] % 16;
		int y = 15 - (msg[i] / 16);

		Vec3 p1 = m * Vec3(x_now  ,  y_bot, 0);
		Vec3 p2 = m * Vec3(x_next ,  y_bot, 0);
		Vec3 p3 = m * Vec3(x_next ,  y_top, 0);
		Vec3 p4 = m * Vec3(x_now  ,  y_top, 0);

		glTexCoord2f( x    * edge_size, y * edge_size);     glVertex3f(p1.x, p1.y, p1.z);
		glTexCoord2f((x+1) * edge_size, y * edge_size);     glVertex3f(p2.x, p2.y, p2.z);
		glTexCoord2f((x+1) * edge_size, (y+1) * edge_size); glVertex3f(p3.x, p3.y, p3.z);
		glTexCoord2f( x    * edge_size, (y+1) * edge_size); glVertex3f(p4.x, p4.y, p4.z);
		++QUADS_DRAWN_THIS_FRAME;
	}
	glEnd();
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
}

void Hud::drawString(const string& msg, float pos_x, float pos_y, float scale, bool background, float alpha) const
{
	glDisable(GL_LIGHTING);
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
		
		totalWidth += 0.05 * charWidth[msg[i]] * 2 * scale;
	}
	
	float x_now     = 0.0f;
	float x_next    = pos_x + 0.05;
	float y_bot     = pos_y;
	float y_top     = pos_y + 0.05 * scale;
	float edge_size = 1./16.;
	
	// draw a darker background box for the text if that was requested
	glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
	glBegin(GL_QUADS);
	if(background)
	{
		glVertex3f(pos_x - 0.01 * scale  , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_top, -1);
		glVertex3f(pos_x - 0.01 * scale  , y_top, -1);
		++QUADS_DRAWN_THIS_FRAME;
	}
	glEnd();
	
	float currentWidth = 0.f;
	float lastWidth    = 0.f;
	
	// reset default colour to white.
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	
	glBegin(GL_QUADS);
	for(size_t i = 0; i < msg.size(); ++i)
	{
		if(msg[i] == '^')
		{
			++i;
			choose_color(msg[i], alpha);
			
			continue;
		}
		
		currentWidth = 0.05 * charWidth[msg[i]];
		
		x_now = x_next + scale * (currentWidth + lastWidth - 0.05f);
		x_next = x_now + 0.05f * scale;
		
		lastWidth = currentWidth;
		
		int x = msg[i] % 16;
		int y = 15 - (msg[i] / 16);

		glTexCoord2f( x    * edge_size, y * edge_size);     glVertex3f(x_now , y_bot, -1);
		glTexCoord2f((x+1) * edge_size, y * edge_size);     glVertex3f(x_next, y_bot, -1);
		glTexCoord2f((x+1) * edge_size, (y+1) * edge_size); glVertex3f(x_next, y_top, -1);
		glTexCoord2f( x    * edge_size, (y+1) * edge_size); glVertex3f(x_now , y_top, -1);
		++QUADS_DRAWN_THIS_FRAME;
	}
	glEnd();
	glDisable(GL_BLEND);
	
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void Hud::drawMinimap() const
{
	static ApoMath apomath = ApoMath();
	
	auto iteratorMyUnit = (*units).find(myID);
	if(iteratorMyUnit == units->end())
		return;

	TextureHandler::getSingleton().bindTexture(0, "");
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	Unit& myUnit = iteratorMyUnit->second;
	
	float map_r = 0.18f;
	float minimap_angle = apomath.getDegrees( myUnit.angle );
	
	float tx = myUnit.position.x.getFloat() / level_max_x;
	float tz = myUnit.position.z.getFloat() / level_max_z;
	
	float unit_x_on_minimap = -2.f * map_r * tx + map_r;
	float unit_z_on_minimap = +2.f * map_r * tz - map_r;
	
	// make minimap rotate along with the user.
	// glTranslatef(+0.74f - unit_x_on_minimap, -0.74f - unit_z_on_minimap, 0.0f);
	glTranslatef(+0.74f, -0.74f, 0.0f);
	glRotatef(-minimap_angle, 0.0f, 0.0f, 1.0f);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	/*
	glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
	glBegin(GL_QUADS);
	glVertex3f(-map_r, -map_r, 0.f);
	glVertex3f(+map_r, -map_r, 0.f);
	glVertex3f(+map_r, +map_r, 0.f);
	glVertex3f(-map_r, +map_r, 0.f);
	++QUADS_DRAWN_THIS_FRAME;
	glEnd();
	*/
	
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	for(auto it = units->begin(); it != units->end(); ++it)
	{
		const int id = it->first;
		
		float r = 1.0f;
		float g = 0.0f;
		float b = 0.0f;
		
		if(myID == id)
		{
			r = 0.0f;
			g = 1.0f;
			b = 0.0f;
		}
		
		const Unit& u = it->second;
		const Location& loc = u.getPosition();
		
		float x = -(loc.x.getFloat() / level_max_x) * 2 * map_r + map_r - unit_x_on_minimap;
		float z = +(loc.z.getFloat() / level_max_z) * 2 * map_r - map_r - unit_z_on_minimap;
		
		glColor3f(r, g, b); glVertex3f(x, z, 0.0f);
	}
	glEnd();
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

