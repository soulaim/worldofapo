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
	game.init();
}

void Localplayer::readConfig()
{
	game.readConfig();
}


void Localplayer::handleWorldEvents()
{
	game.handleWorldEvents();
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
	game.client_tick_local();
}

void Localplayer::process_game_input()
{
	game.process_game_input();
}

void Localplayer::playSound(const string& name, Location& position)
{
	game.playSound(name, position);
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
	game.client_tick();
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

