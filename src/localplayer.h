#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "game.h"
#include "world.h"
#include "userio.h"
#include "graphics.h"
#include "ordercontainer.h"
#include "fps_manager.h"
#include "order.h"
#include "playerinfo.h"
#include "gamesound.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>


class Localplayer
{
public:
	Localplayer();

	void client_tick();
	void menu_tick();
	void draw();
	void init();

	bool in_menu() const;
private:
	void joinInternetGame(const std::string&);
	void endGame();
	
	void reset();
	void readConfig();
	
	void handleWorldEvents();
	void handleServerMessage(const Order&);
	void processClientMsgs();
	void camera_handling();
	int connectMenu();

	void check_messages_from_server();
	void handleClientLocalInput();
	void client_tick_local();
	void process_game_input();
	
	// fully 3D single channel sounds! :DD
	void playSound(const string& name, Location& position);
	
	void enableGrab();
	void disableGrab();

private:
	Game game;
	Graphics view;
};


#endif

