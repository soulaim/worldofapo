#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "game.h"
#include "world.h"
#include "visualworld.h"
#include "ordercontainer.h"
#include "fps_manager.h"
#include "order.h"
#include "playerinfo.h"
#include "gamesound.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>

class UserIO;
class Graphics;
class Hud;

class Localplayer
{
public:
	Localplayer(Graphics*, UserIO*, Hud*);

	bool client_tick();

	void draw();
	void init();

	bool joinInternetGame(const std::string&);

private:
	void endGame();
	
	void reset();
	
	void handleWorldEvents();
	void camera_handling();

	bool handleClientLocalInput();
	void process_sent_game_input();
	
	// fully 3D single channel sounds! :DD
	void playSound(const std::string& name, const Location& position);
	
	void enableGrab();
	void disableGrab();

private:
	int client_input_state;
	std::string clientCommand;

	Hud* hud;
	Graphics* view;
	UserIO* userio;

	Game game;
	GameSound soundsystem;
	VisualWorld visualworld;
	World world;
};


#endif

