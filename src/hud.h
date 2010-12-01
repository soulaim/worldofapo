#ifndef HUD_H
#define HUD_H

#include <string>
#include <vector>
#include <map>

#include "playerinfo.h"
#include "unit.h"
#include "location.h"
#include "viewmessage.h"
#include "frustum/vec3.h"

class Hud
{
public:
	Hud();

	void setLocalPlayerName(const std::string&);
	void setLocalPlayerHP(const int);
	void setPlayerInfo(std::map<int,PlayerInfo>* pInfo);
	void setZombiesLeft(int);
	void setLocalPlayerKills(const int k);
	void setLocalPlayerDeaths(const int d);
	void pushMessage(const std::string&);
	void setCurrentClientCommand(const std::string&);
	void setTime(unsigned);
	void world_tick();
	void setLevelSize(const FixedPoint& x, const FixedPoint& z);
	void setUnitsMap(std::map<int, Unit>* units);
	void setLocalPlayerID(int _myID);
	void setShowStats(bool);

	void drawString(const std::string&, float pos_x = -1.0f, float pos_y = -1.0f, float scale = 1.0f, bool background = false, float alpha = 1.0f) const;
	void drawMessages();
	void drawCrossHair() const;
	void drawStatusBar() const;
	void drawZombiesLeft() const;
	void drawBanner() const;
	void drawMinimap() const;
	void drawFPS();
	void drawStats() const;
	void drawAmmo() const;
	void draw(bool firstPerson);

	void draw3Dstring(const std::string& msg, const Vec3& pos, float x_angle, float y_angle, int team = 0) const;

private:
	FixedPoint level_max_z;
	FixedPoint level_max_x;

	std::string currentClientCommand;
	std::vector<ViewMessage> viewMessages;
	std::string kills;
	std::string deaths;
	std::string health;
	std::string plr_name;
	bool showStats;
	int zombieCount;
	int plr_ID;
	
	Location unit_location;
	unsigned currentTime;
	int world_ticks;

	std::map<int, PlayerInfo>* Players;
	std::map<int, Unit>* units;
	int myID;
};

#endif

