

#include <algorithm>

#include "localplayer.h"
#include "game.h"
#include "logger.h"

using namespace std;


void Localplayer::playSound(const string& name, Location& position)
{
	// play sounds!
	if(game.myID != -1)
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
	int wheel_status = userio.getMouseWheelScrolled();
	if (wheel_status == 1)
		view.mouseUp();
	if (wheel_status == 2)
		view.mouseDown();
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
	int keyState = userio.getGameInput();
	if (client_input_state & 2)
		keyState = 0;
	int x, y;
	
	userio.getMouseChange(x, y);
	int mousepress = userio.getMousePress();

	view.updateInput(keyState); // Make only "small" local changes like change camera angle.

	game.set_current_frame_input(keyState, x, y, mousepress);
}

void Localplayer::handleClientLocalInput()
{
	camera_handling();
	
	string key = userio.getSingleKey();
	
	if(key.size() == 0)
		return;
	
	if(key == "return")
		client_input_state ^= 2;
	else if(key == "f11")
		view.toggleFullscreen();
	else if(key == "f10")
		view.toggleLightingStatus();
	else if(key == "f9")
		world.show_errors ^= 1;
	
	if(client_input_state & 2) // chat message
	{
		string nick;
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
		view.setCurrentClientCommand(nick);
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
			view.setCurrentClientCommand(clientCommand);
		}
		
		if(key == "escape")
		{
			game.endGame();
			
			// then proceed with local shutdown.
			cerr << "User pressed ESC, shutting down." << endl;
			SDL_Quit();
			exit(0);
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
}




// world events are local events generated by the world simulation.
void Localplayer::handleWorldEvents()
{
	if(game.myID != -1)
	{
		view.setLocalPlayerName(game.Players[game.myID].name);
		view.setLocalPlayerHP(world.units[game.myID].hitpoints);
	}

	view.setZombiesLeft(world.getZombies());
	view.setHumanPositions(world.humanPositions());
	
	// deliver any world message events to graphics structure, and erase them from world data.
	for(size_t i = 0; i < world.worldMessages.size(); ++i)
	{
		view.pushMessage(world.worldMessages[i]);
	}
	world.worldMessages.clear();
	
	// handle any world events <-> graphics structure
	for(size_t i = 0; i < world.events.size(); ++i)
	{
		WorldEvent& event = world.events[i];
		if(event.type == World::DAMAGE_BULLET)
			view.genParticles(event.position, event.velocity, 5*4, 0.3, 0.4f, 0.6f, 0.2f, 0.2f);
		else if(event.type == World::DAMAGE_DEVOUR)
			view.genParticles(event.position, event.velocity, 5*9, 0.7, 0.4f, 0.9f, 0.2f, 0.2f);
		else if(event.type == World::DEATH_ENEMY)
		{
			playSound("alien_death", event.position);
			view.genParticles(event.position, event.velocity, 5*30, 2.0, 1.0f, 0.1f, 0.5f, 0.2f);
		}
		else if(event.type == World::DEATH_PLAYER)
		{
			playSound("player_death", event.position);
			view.genParticles(event.position, event.velocity, 5*30, 2.0, 1.0f, 1.0f, 0.2f, 0.2f);
		}
		else if(event.type == World::DEATH_ENEMY)
		{
			if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
			{
					game.Players[event.actor_id].kills++;
			}
		}
		else if(event.type == World::DEATH_PLAYER)
		{
			if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
				game.Players[event.actor_id].kills++;
			if( (world.units.find(event.target_id) != world.units.end()) && world.units[event.target_id].human())
				game.Players[event.target_id].deaths++;
		}
		else if(event.type == World::CENTER_CAMERA)
		{
			if( (world.units.find(event.actor_id) != world.units.end()) )
			{
				cerr << "Binding camera to unit " << event.actor_id << endl;
				view.bindCamera(&world.units[event.actor_id]);
			}
		}
		else
		{
			cerr << "UNKNOWN world EVENT OCCURRED" << endl;
		}
	}
	
	world.events.clear();
	view.setLocalPlayerKills(game.Players[game.myID].kills);
	view.setLocalPlayerDeaths(game.Players[game.myID].deaths);
}

