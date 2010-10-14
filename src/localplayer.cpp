#include "localplayer.h"

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

Localplayer::Localplayer():
	game(&view)
{
}


void Localplayer::joinInternetGame(const std::string& hostname)
{
	game.joinInternetGame(hostname);
}

void Localplayer::endGame()
{
	game.endGame();
}


void Localplayer::reset()
{
	game.reset();
}

void Localplayer::init()
{
	soundsystem.init();
	game.init();
}

void Localplayer::readConfig()
{
	game.readConfig();
}


void Localplayer::handleServerMessage(const Order& server_msg)
{
	game.handleServerMessage(server_msg);
}

void Localplayer::processClientMsgs()
{
	game.processClientMsgs();
}

void Localplayer::camera_handling()
{
	game.camera_handling();
}

int Localplayer::connectMenu()
{
	return game.connectMenu();
}

void Localplayer::check_messages_from_server()
{
	game.check_messages_from_server();
}

void Localplayer::handleClientLocalInput()
{
	game.handleClientLocalInput();
}

void Localplayer::client_tick_local()
{
	if(game.client_tick_local())
	{
		handleWorldEvents();
		
		for(map<int, Unit>::iterator iter = game.world.units.begin(); iter != game.world.units.end(); iter++)
			playSound(iter->second.soundInfo, iter->second.position);
	}
}

void Localplayer::process_game_input()
{
	game.process_game_input();
}

void Localplayer::enableGrab()
{
	game.enableGrab();
}

void Localplayer::disableGrab()
{
	game.disableGrab();
}


void Localplayer::client_tick()
{
	game.check_messages_from_server();
	game.handleClientLocalInput();

	// if state_descriptor == 0, the userIO
	// is used by HOST functions. Do not interfere.
	if( ((game.state == "client") || (game.state_descriptor != 0)) && (game.client_state & 1))  
	{
		client_tick_local();
	}
}

void Localplayer::menu_tick()
{
	game.menu_tick();
}

void Localplayer::draw()
{
	view.setTime( SDL_GetTicks() );
	if((game.world.units.find(game.myID) != game.world.units.end()) && (game.myID >= 0)) // TODO: why do we need myID?
	{
		game.world.viewTick();
		view.tick();
		view.draw(game.world.models, game.world.lvl, game.world.units);
	}
}

bool Localplayer::in_menu() const
{
	return game.state == "menu";
}

