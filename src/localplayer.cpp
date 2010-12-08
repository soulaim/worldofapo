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
#include "userio.h"
#include "graphics.h"
#include "hud.h"
#include "window.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <sstream>

using namespace std;

Localplayer::Localplayer(Graphics* g, UserIO* u, Hud* h, Window* w):
	client_input_state(0),
	hud(h),
	view(g),
	userio(u),
	window(w),
	game(&world),
	world(&visualworld)
{
	cerr << "Localplayer(...)" << endl;

	hud->setLevelSize(world.lvl.max_x(), world.lvl.max_z());
	need_to_tick_world = false;
}

// returns true on success.
bool Localplayer::joinInternetGame(const std::string& hostname)
{
	world.buildTerrain(1); // TODO: Parameters should be obtained from server.
	visualworld.decorate(world.lvl);
	
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
	cerr << "Localplayer::init()" << endl;

	load("localplayer.conf");

	soundsystem.init();
	game.init();
	userio->init();
	view->init(visualworld.camera);
	
	hud->setPlayerInfo(&game.Players);
	hud->setUnitsMap(&world.units);

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
			need_to_tick_world = true;
			
			process_sent_game_input();
			game.process_received_game_input();
			
			game.TICK();
			handleWorldEvents();
			
			hud->world_tick();
			view->world_tick(world.lvl, visualworld.lights);
		}
		else
		{
			need_to_tick_world = false;
		}
	}
	return stop;
}

void Localplayer::draw()
{
	hud->setTime( SDL_GetTicks() );
	if((world.units.find(game.myID) != world.units.end()) && (game.myID >= 0)) // TODO: why do we need myID?
	{
		visualworld.viewTick(world.units, world.projectiles, world.currentWorldFrame);
		view->tick();

#ifndef __APPLE__
		if(!need_to_tick_world)
			// TODO: is this apple check really necessary??
#endif
		{
			// if we didn't need to tick the world right now, then there should be time to draw the scene.
			view->draw(world.lvl, visualworld, world.octree, world.projectiles, world.units);
		}
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

void Localplayer::process_sent_game_input()
{
	int keyState = userio->getGameInput();
	if (client_input_state & 2)
		keyState = 0;
	int x, y;
	
	userio->getMouseChange(x, y);
	int mousepress = userio->getMousePress();

	view->updateInput(keyState); // Make only "small" local changes like change camera angle.
	hud->setShowStats(keyState & (1 << 31));

	x *= intVals["sensitivity"];
	y *= intVals["sensitivity"];

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
	else if(key == "f7")
	{
		std::cerr << "RELOADING SHADERS" << std::endl;
		view->reload_shaders();
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
		hud->setCurrentClientCommand(nick);
	}
	else
	{
		if(key == "pause")
		{
			window->screenshot();
		}
		else if(key == "return") // handle client local command
		{
			if(clientCommand.size() > 0)
			{
				stringstream msg_ss(clientCommand);
				string word;
				int value;
				
				msg_ss >> word >> value;
				
				if(world.intVals.find(word) != world.intVals.end())
				{
					world.add_message("^Gvalue set");
					world.intVals[word] = value;
				}
				else if(view->intVals.find(word) != view->intVals.end())
				{
					world.add_message("^Gvalue set");
					view->intVals[word] = value;
				}
				else if(intVals.find(word) != intVals.end())
				{
					world.add_message("^Gvalue set");
					intVals[word] = value;
				}
				else if(visualworld.intVals.find(word) != visualworld.intVals.end())
				{
					world.add_message("^Gvalue set");
					visualworld.intVals[word] = value;
				}
				else
				{
					game.send_chat_message(clientCommand);
				}
			}
			
			clientCommand = "";
			hud->setCurrentClientCommand(clientCommand);
		}
		else if(key == "escape")
		{
			game.endGame();
			
			// then proceed with local shutdown.
			std::cerr << "User pressed ESC, shutting down." << std::endl;
			return false;
		}
		else if(key == "g")
		{
			if(client_input_state & 4)
			{
				window->enable_grab();
			}
			else
			{
				window->disable_grab();
			}
			client_input_state ^= 4;
		}
		else if(key == "n")
		{
			auto iter = world.units.find(visualworld.camera.unit_id);
			if(iter == world.units.end())
				iter = world.units.begin();
			else
			{
				iter++;
				if(iter == world.units.end())
					iter = world.units.begin();
			}
			
			if(iter != world.units.end())
			{
				visualworld.bindCamera(&world.units[iter->first]);
				std::stringstream ss_msg;
				ss_msg << "Bound camera to " << iter->second.name << " with unitID " << iter->first;
				world.add_message(ss_msg.str());
			}
		}
		else if(key == "p")
		{
			auto iter = world.units.find(visualworld.camera.unit_id);
			if(iter != world.units.begin())
			{
				iter--;
				visualworld.bindCamera(&world.units[iter->first]);
				std::stringstream ss_msg;
				ss_msg << "Bound camera to " << iter->second.name << " with unitID " << iter->first;
				world.add_message(ss_msg.str());
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
		hud->setLocalPlayerID(game.myID);
		hud->setLocalPlayerName(game.Players[game.myID].name);
		hud->setLocalPlayerHP(world.units.find(game.myID)->second.hitpoints);
	}

	hud->setZombiesLeft(world.getZombies());
	
	// deliver any world message events to graphics structure, and erase them from world data.
	for(size_t i = 0; i < visualworld.worldMessages.size(); ++i)
	{
		hud->pushMessage(visualworld.worldMessages[i]);
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
					visualworld.bindCamera(&world.units.find(event.actor_id)->second);
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
	hud->setLocalPlayerKills(game.Players[game.myID].kills);
	hud->setLocalPlayerDeaths(game.Players[game.myID].deaths);

	for(auto iter = world.units.begin(); iter != world.units.end(); iter++)
	{
		playSound(iter->second.soundInfo, iter->second.getPosition());
	}
}

