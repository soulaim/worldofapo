

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
		Location reference_point = game.world.units[game.myID].position;
		
		FixedPoint distance = (reference_point - position).length();
		
		if(distance < FixedPoint(1))
			distance = FixedPoint(1);
		
		
		// play local player's unit's sound effect
		if(name == "walk")
		{
//			if(simulRules.currentFrame % 15 == 0)
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



bool Game::client_tick_local()
{
	// this is acceptable because the size is guaranteed to be insignificantly small
	sort(UnitInput.begin(), UnitInput.end());
	
	// handle any server commands intended for this frame
	while((UnitInput.back().plr_id == -1) && (UnitInput.back().frameID == simulRules.currentFrame))
	{
		Order server_command = UnitInput.back();
		UnitInput.pop_back();
		handleServerMessage(server_command);
	}
	
	if( (simulRules.currentFrame < simulRules.allowedFrame) && (fps_world.need_to_draw(SDL_GetTicks()) == 1) )
	{
		if( (UnitInput.back().plr_id == -1) && (UnitInput.back().frameID != simulRules.currentFrame) )
			cerr << "ERROR: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;
		
		fps_world.insert();
		process_received_game_input(); // TODO: check if everything works, process (sent) game input used to be here.
		
		// run simulation for one WorldFrame
		world.worldTick(simulRules.currentFrame);
		simulRules.currentFrame++;
		view->world_tick();

		return true;
	}

	return false;
}



void Localplayer::process_sent_game_input()
{
	int keyState = userio.getGameInput();
	if (game.client_state & 2)
		keyState = 0;
	int x, y;
	
	userio.getMouseChange(x, y);

	view.updateInput(keyState, x, y); // Make only "small" local changes like change camera angle.

	// TODO: move to game.
	int frame = game.simulRules.currentFrame + game.simulRules.frameSkip * game.simulRules.windowSize;
	
	if(game.myID >= 0)
	{
		stringstream inputMsg;
		string msg;
		inputMsg << "1 " << game.myID << " " << frame << " " << keyState << " " << x << " " << y << " " << userio.getMousePress() << "#";
		msg = inputMsg.str();
		game.clientSocket.write(msg);
	}
}

void Game::process_received_game_input()
{
	Logger log;
	// update commands of player controlled characters
	while(UnitInput.back().frameID == simulRules.currentFrame)
	{
		Order tmp = UnitInput.back();
		UnitInput.pop_back();
		
		// log all processed game data affecting commands in the order of processing
		log.print(tmp.copyOrder());
		
		if(tmp.plr_id == -1)
		{
			cerr << "MOTHERFUCKER FUCKING FUCK YOU MAN?= JUST FUCK YOU!!" << endl;
			break;
		}
		
		world.units[tmp.plr_id].updateInput(tmp.keyState, tmp.mousex, tmp.mousey, tmp.mouseButtons);
	}
	
	log.print("\n");
}


void Localplayer::handleClientLocalInput()
{
	camera_handling();
	
	string key = userio.getSingleKey();
	
	if(key.size() == 0)
		return;
	
	if(key == "return")
		game.client_state ^= 2;
	else if(key == "f11")
		view.toggleFullscreen();
	else if(key == "f10")
		view.toggleLightingStatus();
	else if(key == "f9")
		game.world.show_errors ^= 1;
	
	if(game.client_state & 2) // chat message
	{
		string nick;
		nick.append("<");
		nick.append(game.Players[game.myID].name);
		nick.append("> ");
		
		if(key.size() == 1)
		{
			game.clientCommand.append(key);
		}
		else if(key == "backspace" && game.clientCommand.size() > 0)
			game.clientCommand.resize(game.clientCommand.size()-1);
		
		else if(key == "escape")
		{
			game.client_state ^= 2;
			game.clientCommand = "";
			nick = "";
		}
		else if(key == "space")
			game.clientCommand.append(" ");
		
		nick.append(game.clientCommand);
		view.setCurrentClientCommand(nick);
	}
	else
	{
		
		if(key == "return") // handle client local command
		{
			if(game.clientCommand.size() > 0)
			{
				stringstream tmp_msg;
				tmp_msg << "3 " << game.myID << " " << game.clientCommand << "#";
				game.clientSocket.write(tmp_msg.str());
			}
			
			game.clientCommand = "";
			view.setCurrentClientCommand(game.clientCommand);
		}
		
		if(key == "escape")
		{
			// shutdown the connection first, so the others can continue playing in peace.
			game.clientSocket.closeConnection();
			
			// then proceed with local shutdown.
			cerr << "User pressed ESC, shutting down." << endl;
			SDL_Quit();
			exit(0);
		}
		
		if(key == "g")
		{
			if (game.client_state & 4)
				enableGrab();
			else
				disableGrab();
			game.client_state ^= 4;
		}
	}
}




// world events are local events generated by the world simulation.
void Localplayer::handleWorldEvents()
{
	if(game.myID != -1)
	{
		view.setLocalPlayerName(game.Players[game.myID].name);
		view.setLocalPlayerHP(game.world.units[game.myID].hitpoints);
	}

	view.setZombiesLeft(game.world.getZombies());
	view.setHumanPositions(game.world.humanPositions());
	
	// deliver any world message events to graphics structure, and erase them from game.world data.
	for(size_t i = 0; i < game.world.worldMessages.size(); ++i)
	{
		view.pushMessage(game.world.worldMessages[i]);
	}
	game.world.worldMessages.clear();
	
	// handle any world events <-> graphics structure
	for(size_t i = 0; i < game.world.events.size(); ++i)
	{
		WorldEvent& event = game.world.events[i];
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
		else
			cerr << "UNKNOWN game.world EVENT OCCURRED" << endl;
		
		if(event.type == World::DEATH_ENEMY)
		{
			if( (game.world.units.find(event.actor_id) != game.world.units.end()) && game.world.units[event.actor_id].human())
			{
					game.Players[event.actor_id].kills++;
			}
		}
		
		if(event.type == World::DEATH_PLAYER)
		{
			if( (game.world.units.find(event.actor_id) != game.world.units.end()) && game.world.units[event.actor_id].human())
				game.Players[event.actor_id].kills++;
			if( (game.world.units.find(event.target_id) != game.world.units.end()) && game.world.units[event.target_id].human())
				game.Players[event.target_id].deaths++;
		}
		
	}
	
	game.world.events.clear();
	view.setLocalPlayerKills(game.Players[game.myID].kills);
	view.setLocalPlayerDeaths(game.Players[game.myID].deaths);
}

