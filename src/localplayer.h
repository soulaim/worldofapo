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
	Localplayer(Graphics*, UserIO*);

	bool client_tick();

	void draw();
	void init();

	bool joinInternetGame(const std::string&);

private:
	void endGame();
	
	void reset();
	
	void handleWorldEvents();
	void camera_handling();

	void handleClientLocalInput();
	void process_sent_game_input();
	
	// fully 3D single channel sounds! :DD
	void playSound(const std::string& name, Location& position);
	
	void enableGrab();
	void disableGrab();

private:
	int client_input_state;
	std::string clientCommand;

	Game game;
	Graphics* view;
	UserIO* userio;
	GameSound soundsystem;
	World world;
};


#endif

