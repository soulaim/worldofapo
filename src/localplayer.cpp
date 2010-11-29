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
#include "modelfactory.h"
#include "apomodel.h"

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
	userio(u),
	world(&visualworld)
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

	hud.setPlayerInfo(&game.Players);
	hud.setUnitsMap(&world.units);

	// TODO: Should not be done here? FIX
	ApoModel::loadObjects("models/model.parts"); // TODO: this is ugly, we shouldn't have to know about apomodel here.
	ApoModel::loadObjects("models/bullet.parts"); // TODO: modelfactory should call these somehow or something.
	ModelFactory::load(World::BULLET_MODEL, "models/bullet.bones", "");
	ModelFactory::load(World::PLAYER_MODEL, "models/model.skeleton", "marine");
	ModelFactory::load(World::INVISIBLE_MODEL, "", "");
	Animation::load("models/model.animation");
	Animation::load("models/skeleani.animation");

	TextureHandler::getSingleton().createTextures("data/textures.txt");
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
			handleWorldEvents();
			
			hud.world_tick();
			view->world_tick(world.lvl, visualworld.lights);
		}
	}
	return stop;
}

void Localplayer::draw()
{
	hud.setTime( SDL_GetTicks() );
	if((world.units.find(game.myID) != world.units.end()) && (game.myID >= 0)) // TODO: why do we need myID?
	{
		visualworld.viewTick(world.units, world.projectiles, world.currentWorldFrame);
		view->tick();
		view->draw(world.lvl, visualworld, &hud, world.octree, world.projectiles, world.units);
	}
}

void Localplayer::playSound(const std::string& name, const Location& position)
{
	// play sounds!
	if(game.myID >= 0)
	{
		const Location& reference_point = world.units.find(game.myID)->second.getPosition();
		
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
	if(wheel_status == UserIO::SCROLL_UP)
	{
		view->zoom_in();
	}

	if(wheel_status == UserIO::SCROLL_DOWN)
	{
		view->zoom_out();
	}
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
	hud.setShowStats(keyState & (1 << 31));

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
	else if(key == "f5")
	{
		std::cerr << "LOADING SHADERS" << std::endl;
		view->initShaders();
	}
	else if(key == "f6")
	{
		std::cerr << "RELEASING SHADERS" << std::endl;
		view->releaseShaders();
	}
	else if(key == "f7")
	{
		std::cerr << "RELOADING SHADERS" << std::endl;
		view->initShaders();
	}
	
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
		
		if(key == "n")
		{
			auto iter = world.units.find(game.myID);
			iter++;
			if(iter != world.units.end())
			{
				game.myID = iter->first;
				view->bindCamera(&world.units[game.myID]);
				world.add_message("Increased value of myID!");
			}
		}
		
		if(key == "p")
		{
			auto iter = world.units.find(game.myID);
			if(iter != world.units.begin())
			{
				iter--;
				game.myID = iter->first;
				view->bindCamera(&world.units[game.myID]);
				world.add_message("Decreased value of myID!");
			}
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
		hud.setLocalPlayerHP(world.units.find(game.myID)->second.hitpoints);
	}

	hud.setZombiesLeft(world.getZombies());
	
	// deliver any world message events to graphics structure, and erase them from world data.
	for(size_t i = 0; i < visualworld.worldMessages.size(); ++i)
	{
		hud.pushMessage(visualworld.worldMessages[i]);
	}
	visualworld.worldMessages.clear();
	
	// handle any world events <-> graphics structure
	for(size_t i = 0; i < visualworld.events.size(); ++i)
	{
		int scale = 1000;
		int max_rand = 500;
		
		WorldEvent& event = visualworld.events[i];
		switch(event.type)
		{
			case WorldEvent::DAMAGE_BULLET:
			{
				std::stringstream ss;
				int x = (rand() % 4);
				ss << "hit" << x;
				playSound(ss.str(), event.t_position);
				
				Location bulletDirection = event.a_velocity;
				bulletDirection.normalize();
				
				visualworld.genParticleEmitter(event.a_position, bulletDirection, 5, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED");
				break;
			}
			case WorldEvent::DAMAGE_DEVOUR:
			{
				playSound("hit0", event.t_position);
				visualworld.genParticleEmitter(event.t_position, event.t_velocity, 5, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED");
				break;
			}
			case WorldEvent::DEATH_ENEMY:
			{
				playSound("alien_death", event.t_position);

				visualworld.genParticleEmitter(event.t_position, event.t_velocity, 15, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED", 2000, 25);

				if( (world.units.find(event.actor_id) != world.units.end()) && world.units.find(event.actor_id)->second.human())
				{
					game.Players[event.actor_id].kills++;
				}
				break;
			}
			case WorldEvent::DEATH_PLAYER:
			{
				playSound("player_death", event.t_position);
				visualworld.genParticleEmitter(event.t_position, event.t_velocity, 15, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED", 2000, 25);
				
				/*
				if(event.actor_id == game.myID)
				{
					playSound("taunt", event.t_position);
				}
				*/
				
				if( (world.units.find(event.actor_id) != world.units.end()) && world.units.find(event.actor_id)->second.human())
				{
					game.Players[event.actor_id].kills++;
					if(game.Players[event.actor_id].kills > 9) // this way EVERYONE can be dominating :D
						playSound("domination", event.t_position);
				}
				
				if( (world.units.find(event.target_id) != world.units.end()) && world.units.find(event.target_id)->second.human())
				{
					game.Players[event.target_id].deaths++;
				}
				break;
			}
			case WorldEvent::CENTER_CAMERA:
			{
				if( (world.units.find(event.actor_id) != world.units.end()) )
				{
					std::cerr << "Binding camera to unit " << event.actor_id << std::endl;
					view->bindCamera(&world.units.find(event.actor_id)->second);
				}
				break;
			}
			default:
			{
				std::cerr << "UNKNOWN world EVENT OCCURRED" << std::endl;
			}
		}
	}
	
	visualworld.events.clear();
	hud.setLocalPlayerKills(game.Players[game.myID].kills);
	hud.setLocalPlayerDeaths(game.Players[game.myID].deaths);

	for(auto iter = world.units.begin(); iter != world.units.end(); iter++)
	{
		playSound(iter->second.soundInfo, iter->second.getPosition());
	}
}

