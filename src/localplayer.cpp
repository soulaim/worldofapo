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
#include "animation.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>

Localplayer::Localplayer(Graphics* g, UserIO* u):
	client_input_state(0),
	game(&world),
	view(g),
	userio(u)
{
	hud.setLevelSize(world.lvl.max_x(), world.lvl.max_z());
}


bool Localplayer::joinInternetGame(const std::string& hostname)
{
	return game.joinInternetGame(hostname);
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
	userio->init();

	view->loadObjects("models/model.parts");
	view->loadObjects("models/bullet.parts");
	Animation::load("models/model.animation");
	Animation::load("models/skeleani.animation");
	
	hud.setPlayerInfo(&game.Players);
	hud.setUnitsMap(&world.units);

	// TODO: Should not be done here? FIX
	TextureHandler::getSingleton().createTexture("grass", "data/grass.png");
	TextureHandler::getSingleton().createTexture("highground", "data/highground.png");
	TextureHandler::getSingleton().createTexture("mountain", "data/hill.png");
	TextureHandler::getSingleton().createTexture("marine", "models/texture_marine.png");
}

bool Localplayer::client_tick()
{
	bool stop = game.check_messages_from_server();
	game.processClientMsgs();

	stop = stop || !handleClientLocalInput();

	if(!stop && !game.paused())
	{
		if(game.client_tick_local())
		{
			process_sent_game_input();
			game.process_received_game_input();
			
			game.TICK();

			hud.world_tick();
			view->world_tick(world.lvl);
			handleWorldEvents();
		}
	}
	return stop;
}

void Localplayer::draw()
{
	hud.setTime( SDL_GetTicks() );
	if((world.units.find(game.myID) != world.units.end()) && (game.myID >= 0)) // TODO: why do we need myID?
	{
		world.viewTick();
		view->tick();
		view->draw(world.models, world.lvl, world.units, world.lights, world.o, &hud, world.medikits, world.projectiles);
	}
}

void Localplayer::playSound(const std::string& name, Location& position)
{
	// play sounds!
	if(game.myID >= 0)
	{
		Location reference_point = world.units[game.myID].position;
		
		FixedPoint distance = (reference_point - position).length();
		
		if(distance < FixedPoint(1))
			distance = FixedPoint(1);
		
		
		// play local player's unit's sound effect
		if(name == "walk")
		{
			static int frame = 0;
			++frame;
			if(frame % 15 == 0)
			{
				soundsystem.playEffect(name, distance.getFloat(), 100000);
			}
		}
		else
		{
			soundsystem.playEffect(name, distance.getFloat(), 100000);
		}
	}
}


void Localplayer::camera_handling()
{
	int wheel_status = userio->getMouseWheelScrolled();
	if (wheel_status == 1)
		view->mouseUp();
	if (wheel_status == 2)
		view->mouseDown();
}

void Localplayer::enableGrab()
{
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(0);
}

void Localplayer::disableGrab()
{
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(1);
}

void Localplayer::process_sent_game_input()
{
	int keyState = userio->getGameInput();
	if (client_input_state & 2)
		keyState = 0;
	int x, y;
	
	userio->getMouseChange(x, y);
	int mousepress = userio->getMousePress();

	view->updateInput(keyState); // Make only "small" local changes like change camera angle.

	game.set_current_frame_input(keyState, x, y, mousepress);
}

bool Localplayer::handleClientLocalInput()
{
	camera_handling();
	
	std::string key = userio->getSingleKey();
	
	if(key.size() == 0)
		return true;
	
	if(key == "return")
		client_input_state ^= 2;
	else if(key == "f11")
		view->toggleFullscreen();
	else if(key == "f10")
		view->toggleLightingStatus();
	else if(key == "f8")
		view->toggleWireframeStatus();
	else if(key == "f9")
		world.show_errors ^= 1;
	
	if(client_input_state & 2) // chat message
	{
		std::string nick;
		nick.append("<");
		nick.append(game.Players[game.myID].name);
		nick.append("> ");
		
		if(key.size() == 1)
		{
			clientCommand.append(key);
		}
		else if(key == "backspace" && clientCommand.size() > 0)
			clientCommand.resize(clientCommand.size()-1);
		
		else if(key == "escape")
		{
			client_input_state ^= 2;
			clientCommand = "";
			nick = "";
		}
		else if(key == "space")
			clientCommand.append(" ");
		
		nick.append(clientCommand);
		hud.setCurrentClientCommand(nick);
	}
	else
	{
		
		if(key == "return") // handle client local command
		{
			if(clientCommand.size() > 0)
			{
				game.send_chat_message(clientCommand);
			}
			
			clientCommand = "";
			hud.setCurrentClientCommand(clientCommand);
		}
		
		if(key == "escape")
		{
			game.endGame();
			
			// then proceed with local shutdown.
			std::cerr << "User pressed ESC, shutting down." << std::endl;
			return false;
		}
		
		if(key == "g")
		{
			if (client_input_state & 4)
				enableGrab();
			else
				disableGrab();
			client_input_state ^= 4;
		}
	}
	return true;
}




// world events are local events generated by the world simulation.
void Localplayer::handleWorldEvents()
{
	if(game.myID >= 0)
	{
		hud.setLocalPlayerID(game.myID);
		hud.setLocalPlayerName(game.Players[game.myID].name);
		hud.setLocalPlayerHP(world.units[game.myID].hitpoints);
	}

	hud.setZombiesLeft(world.getZombies());
	
	// deliver any world message events to graphics structure, and erase them from world data.
	for(size_t i = 0; i < world.worldMessages.size(); ++i)
	{
		hud.pushMessage(world.worldMessages[i]);
	}
	world.worldMessages.clear();
	
	// handle any world events <-> graphics structure
	for(size_t i = 0; i < world.events.size(); ++i)
	{
		WorldEvent& event = world.events[i];
		switch(event.type)
		{
			case World::DAMAGE_BULLET:
			{
				std::stringstream ss;
				int x = (rand() % 4);
				ss << "hit" << x;
				playSound(ss.str(), event.position);
				
				view->genParticles(event.position, event.velocity, 60, 0.7, 0.4f, 0.6f, 0.2f, 0.2f);
				break;
			}
			case World::DAMAGE_DEVOUR:
			{
				playSound("hit0", event.position);
				view->genParticles(event.position, event.velocity, 60, 1.5, 0.4f, 0.9f, 0.2f, 0.2f);
				break;
			}
			case World::DEATH_ENEMY:
			{
				playSound("alien_death", event.position);
				view->genParticles(event.position, event.velocity, 300, 2.0, 1.0f, 0.9f, 0.4f, 0.2f);

				if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
				{
					game.Players[event.actor_id].kills++;
				}
				break;
			}
			case World::DEATH_PLAYER:
			{
				playSound("player_death", event.position);
				view->genParticles(event.position, event.velocity, 300, 2.0, 1.0f, 1.0f, 0.2f, 0.2f);

				if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
				{
					game.Players[event.actor_id].kills++;
				}
				if( (world.units.find(event.target_id) != world.units.end()) && world.units[event.target_id].human())
				{
					game.Players[event.target_id].deaths++;
				}
				break;
			}
			case World::CENTER_CAMERA:
			{
				if( (world.units.find(event.actor_id) != world.units.end()) )
				{
					std::cerr << "Binding camera to unit " << event.actor_id << std::endl;
					view->bindCamera(&world.units[event.actor_id]);
				}
				break;
			}
			default:
			{
				std::cerr << "UNKNOWN world EVENT OCCURRED" << std::endl;
			}
		}
	}
	
	world.events.clear();
	hud.setLocalPlayerKills(game.Players[game.myID].kills);
	hud.setLocalPlayerDeaths(game.Players[game.myID].deaths);

	for(auto iter = world.units.begin(); iter != world.units.end(); iter++)
	{
		playSound(iter->second.soundInfo, iter->second.position);
	}
}

