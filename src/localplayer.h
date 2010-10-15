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

	bool client_tick();
	void menu_tick(); // TODO: separate from localplayer?

	void draw();
	void init();

private:
	bool joinInternetGame(const std::string&);
	void endGame();
	
	void reset();
	
	void handleWorldEvents();
	void camera_handling();
	int connectMenu();

	void handleClientLocalInput();
	void process_sent_game_input();
	
	// fully 3D single channel sounds! :DD
	void playSound(const string& name, Location& position);
	
	void enableGrab();
	void disableGrab();

private:
	int client_input_state;
	std::string clientCommand;

	Game game;
	Graphics view;
	GameSound soundsystem;
	UserIO userio;
	World world;
};


#endif

