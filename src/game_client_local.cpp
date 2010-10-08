

#include <algorithm>

#include "game.h"
#include "logger.h"

using namespace std;


void Game::camera_handling()
{
	int wheel_status = userio.getMouseWheelScrolled();
	if (wheel_status == 1)
		view.mouseUp();
	if (wheel_status == 2)
		view.mouseDown();
}

void Game::enableGrab()
{
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(0);
}

void Game::disableGrab()
{
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(1);
}



void Game::client_tick_local()
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
		process_game_input();
		
		// run simulation for one WorldFrame
		world.worldTick(simulRules.currentFrame);
		view.world_tick();
		simulRules.currentFrame++;
		
		handleWorldEvents();
		
		// play sounds!
		if(myID != -1)
		{
			Location reference_point = world.units[myID].position;
			for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
			{
				FixedPoint distance = (reference_point - iter->second.position).length();
				if(distance > FixedPoint(10 * 8))
					continue;
				
				if(distance < FixedPoint(1))
					distance = FixedPoint(1);
				
				
				// play local player's unit's sound effect
				if(iter->second.soundInfo == "walk")
				{
					if(simulRules.currentFrame % 15 == 0)
						soundsystem.playEffect(iter->second.soundInfo, distance.getFloat(), 100000);
				}
				else
				{
					soundsystem.playEffect(iter->second.soundInfo, distance.getFloat(), 100000);
				}
				
			}
		}
	}
}



void Game::process_game_input()
{
	int keyState = userio.getGameInput();
	if (client_state & 2)
		keyState = 0;
	int x, y;
	
	userio.getMouseChange(x, y);
	int frame = simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize;
	
	if(myID >= 0)
	{
		stringstream inputMsg;
		string msg;
		inputMsg << "1 " << myID << " " << frame << " " << keyState << " " << x << " " << y << " " << userio.getMousePress() << "#";
		msg = inputMsg.str();
		clientSocket.write(msg);
	}
	
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
		
		world.units[tmp.plr_id].updateInput(tmp.keyState, tmp.mousex, tmp.mousey, tmp.mouseButton);
	}
	
	log.print("\n");
	view.updateInput(keyState, x, y);
}


void Game::handleClientLocalInput()
{
	camera_handling();
	
	string key = userio.getSingleKey();
	
	if(key.size() == 0)
		return;
	
	if(key == "return")
		client_state ^= 2;
	else if(key == "f11")
		view.toggleFullscreen();
	else if(key == "f10")
		view.toggleLightingStatus();
	else if(key == "p" && state == "host")
	{
		// send spawn monster message
		serverSendMonsterSpawn();
	}
	else if(key == "f9")
		world.show_errors ^= 1;
	
	if(client_state & 2) // chat message
	{
		string nick;
		nick.append("<");
		nick.append(Players[myID].name);
		nick.append("> ");
		
		if(key.size() == 1)
			clientCommand.append(key);
		
		else if(key == "backspace" && clientCommand.size() > 0)
			clientCommand.resize(clientCommand.size()-1);
		
		else if(key == "escape")
		{
			client_state ^= 2;
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
				stringstream tmp_msg;
				tmp_msg << "3 " << myID << " " << clientCommand << "#";
				clientSocket.write(tmp_msg.str());
			}
			
			clientCommand = "";
			view.setCurrentClientCommand(clientCommand);
		}
		
		if(key == "escape")
		{
			// shutdown the connection first, so the others can continue playing in peace.
			clientSocket.closeConnection();
			
			// then proceed with local shutdown.
			cerr << "User pressed ESC, shutting down." << endl;
			SDL_Quit();
			exit(0);
		}
		
		if(key == "g")
		{
			if (client_state & 4)
				enableGrab();
			else
				disableGrab();
			client_state ^= 4;
		}
	}
}




// world events are local events generated by the world simulation.
void Game::handleWorldEvents()
{
	if(myID != -1)
	{
		view.setLocalPlayerName(Players[myID].name);
		view.setLocalPlayerHP(world.units[myID].hitpoints);
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
			view.genParticles(event.position, event.velocity, 5*30, 2.0, 1.0f, 0.1f, 0.5f, 0.2f);
		else if(event.type == World::DEATH_PLAYER)
			view.genParticles(event.position, event.velocity, 5*30, 2.0, 1.0f, 1.0f, 0.2f, 0.2f);
		else
			cerr << "UNKOWN WORLD EVENT OCCURRED" << endl;
		
		if(event.type == World::DEATH_ENEMY)
		{
			if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
			{
					Players[event.actor_id].kills++;
			}
		}
		
		if(event.type == World::DEATH_PLAYER)
		{
			if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
				Players[event.actor_id].kills++;
			if( (world.units.find(event.target_id) != world.units.end()) && world.units[event.target_id].human())
				Players[event.target_id].deaths++;
		}
		
	}
	
	world.events.clear();
	view.setLocalPlayerKills(Players[myID].kills);
	view.setLocalPlayerDeaths(Players[myID].deaths);
	
}

