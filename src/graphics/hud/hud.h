#ifndef HUD_H
#define HUD_H

#include "world/playerinfo.h"
#include "world/objects/unit.h"
#include "world/worldevent.h"
#include "graphics/viewmessage.h"
#include "misc/vec3.h"
#include "misc/hasproperties.h"
#include "misc/messaging_system.h"
#include "graphics/hud/inventoryrenderer.h"
#include "graphics/hud/itempickrenderer.h"
#include "graphics/hud/barRenderer.h"
#include "graphics/hud/textRenderer.h"
#include "graphics/hud/statRenderer.h"

#include <string>
#include <vector>
#include <map>

class Hud : public HasProperties, public MessagingSystem<ChatMessage>
{
	friend class GameView;

public:
	Hud();

    void handle(const ChatMessage& msg);

    // TODO: This is all such a load of crap. Get rid of the shit.
	void reset();
	void setAreaName(const std::string&);
	void setLocalPlayerName(const std::string&);
	void setLocalPlayerHP(const int);
	void setPlayerInfo(std::map<int,PlayerInfo>* pInfo);
	void setLocalPlayerKills(const int k);
	void setLocalPlayerDeaths(const int d);
	void setCurrentClientCommand(const std::string&);
	void setTime(unsigned);
	void world_tick();
	void setLevelSize(const FixedPoint& x, const FixedPoint& z);
	void setUnitsMap(std::map<int, Unit>* units);
	void setLocalPlayerID(int _myID);
	void setShowStats(bool);


	void drawMessages();
	void drawCrossHair() const;
	void drawStatusBar() const;
	void drawBanner() const;
	void drawMinimap() const;
	void drawFPS();
	void drawAmmo() const;
	void draw(bool firstPerson);


    int statViewInput(int dx, int dy, int mousePress, int keyState);

	// TODO: make it work with world-tick data also.
	void insertDebugString(const std::string&); // only per visual frame inserts! not for per world-tick data.
	void clearDebugStrings();

	float getFPS();

private:
	FixedPoint level_max_z;
	FixedPoint level_max_x;

    // why oh why are these not contained in some hudMessages class.
	std::string currentClientCommand;
	std::vector<ViewMessage> viewMessages;
	std::vector<std::string> core_info;

	std::string kills;
	std::string deaths;
	std::string health;
	std::string plr_name;
	std::string area_name;
	bool showStats;

	struct PerformanceData
	{
		PerformanceData();
		void reset();

		long long last_time;
		float fps;
		float world_fps;
		int frames;
		int world_ticks;
	};

	PerformanceData perfData;

	Location unit_location;
	unsigned currentTime;

	std::map<int, PlayerInfo>* Players;
	std::map<int, Unit>* units;

    int myID;

    InventoryRenderer inventoryRenderer;
    ItemPickRenderer itemPickRenderer;
    TextRenderer textRenderer;
    BarRenderer barRenderer;
    StatRenderer statRenderer;
};

#endif

