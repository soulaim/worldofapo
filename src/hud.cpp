#include "graphics.h"

#include <string>
#include <sstream>
#include <map>
#include <iomanip>

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;
extern int QUADS_DRAWN_THIS_FRAME;

void Graphics::setLocalPlayerName(const std::string& name)
{
	plr_name = name;
}

void Graphics::setLocalPlayerHP(const int life)
{
	stringstream ss;
	ss << life;
	health = ss.str();
}

void Graphics::setLocalPlayerKills(const int k)
{
	stringstream ss;
	ss << k;
	kills = ss.str();
}

void Graphics::setLocalPlayerDeaths(const int d)
{
	stringstream ss;
	ss << d;
	deaths = ss.str();
}

void Graphics::drawStatusBar()
{
	stringstream ss;
	ss << plr_name << " hp: " << health << " k/d: " << kills << "/" << deaths;
	drawString(ss.str(), -0.9, 0.9, 2.0, true);
}

void Graphics::drawBanner()
{
	drawString("World of Apo, alpha build 715517");
}

void Graphics::drawZombiesLeft()
{
	stringstream zz;
	zz << "Zombies left: " << zombieCount;
	drawString(zz.str(), 0.2, 0.9, 1.5, true);
}

void Graphics::pushMessage(const string& msg, float r, float g, float b)
{
	viewMessages.push_back(ViewMessage(msg, currentTime));
}

void Graphics::drawMessages()
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

void Graphics::setCurrentClientCommand(const string& cmd)
{
	currentClientCommand = cmd;
}

void Graphics::drawFPS()
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

void Graphics::drawHUD()
{
	if(camera.isFirstPerson())
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


void Graphics::drawStats()
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

void Graphics::setPlayerInfo(map<int,PlayerInfo> *pInfo)
{
	Players = pInfo;
}

