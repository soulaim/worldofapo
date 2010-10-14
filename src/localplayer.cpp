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
#include "texturehandler.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>

Localplayer::Localplayer():
	client_input_state(0),
	game(&world)
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
	userio.init();

	view.loadObjects("data/parts.dat");
	view.megaFuck(); // blah..
	view.setPlayerInfo(&game.Players);

	// TODO: Should not be done here? FIX
	TextureHandler::getSingleton().createTexture("grass", "data/grass.png");
	TextureHandler::getSingleton().createTexture("highground", "data/highground.png");
	TextureHandler::getSingleton().createTexture("mountain", "data/hill.png");
}

bool Localplayer::client_tick()
{
	bool stop = game.check_messages_from_server();

	handleClientLocalInput();
	process_sent_game_input();

	if(!stop && !game.paused())
	{
		if(game.client_tick_local())
		{
			view.world_tick();
			handleWorldEvents();
			
			for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
			{
				playSound(iter->second.soundInfo, iter->second.position);
			}
		}
	}
	return stop;
}

void Localplayer::draw()
{
	view.setTime( SDL_GetTicks() );
	if((world.units.find(game.myID) != world.units.end()) && (game.myID >= 0)) // TODO: why do we need myID?
	{
		world.viewTick();
		view.tick();
		view.draw(world.models, world.lvl, world.units);
	}
}

