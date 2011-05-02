#ifndef HUD_H
#define HUD_H

#include "world/playerinfo.h"
#include "world/unit.h"
#include "graphics/viewmessage.h"
#include "misc/vec3.h"
#include "misc/hasproperties.h"

#include <string>
#include <vector>
#include <map>

class Hud : public HasProperties
{
	friend class GameView;
	
public:
	Hud();

	void setAreaName(const std::string&);
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

	void drawBar(float size, const std::string& start_color, const std::string& end_color, float min_x, float max_x, float min_y, float max_y) const;
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

	void draw3Dstring(const std::string& msg, const vec3<float>& pos, float x_angle, float y_angle, int team = 0) const;
	void draw3DBar(float percentage, const vec3<float>& pos, float x_angle, float y_angle, const std::string& start_color, const std::string& end_color, float scale = 5.0f) const;
	
	// TODO: make it work with world-tick data also.
	void insertDebugString(const std::string&); // only per visual frame inserts! not for per world-tick data.
	void clearDebugStrings();
	
private:
	FixedPoint level_max_z;
	FixedPoint level_max_x;

	std::string currentClientCommand;
	std::vector<ViewMessage> viewMessages;
	std::vector<std::string> core_info;
	
	std::string kills;
	std::string deaths;
	std::string health;
	std::string plr_name;
	std::string area_name;
	bool showStats;
	int plr_ID;
	
	Location unit_location;
	unsigned currentTime;
	int world_ticks;
	int zombieCount;

	std::map<int, PlayerInfo>* Players;
	std::map<int, Unit>* units;
	int myID;
};

#endif

