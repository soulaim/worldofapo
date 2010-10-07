
#include "game.h"

// for debugging
#include "logger.h"

#include <sstream>
#include <algorithm>

using namespace std;


void Game::handleServerMessage(const Order& server_msg)
{
	if(server_msg.serverCommand == 3) // pause!
	{
		client_state = 0;
		cerr << "Pausing the game at frame " << simulRules.currentFrame << endl;
	}
	else if(server_msg.serverCommand == 10)
	{
		world.addUnit(world.nextUnitID(), false);
	}
	
	else if(server_msg.serverCommand == 100) // SOME PLAYER HAS DISCONNECTED
	{
		string viewMessage_str = "[";
		viewMessage_str.append(Players[server_msg.keyState].name);
		viewMessage_str.append("] has disconnected!");
		view.pushMessage(viewMessage_str);
		
		world.units.erase(server_msg.keyState);
		world.models.erase(server_msg.keyState);
		Players.erase(server_msg.keyState);
		simulRules.numPlayers--;
		// BWAHAHAHA...
		
	}
	else if(server_msg.serverCommand == 1) // ADDHERO message
	{
		world.addUnit(server_msg.keyState);
		simulRules.numPlayers++;
		cerr << "Adding a new hero at frame " << simulRules.currentFrame << ", units.size() = " << world.units.size() << ", myID = " << myID << endl;
		view.pushMessage("Hero created!");
		
		cerr << "Creating dummy input for new hero." << endl;
		
		// WE MUST CREATE DUMMY INPUT FOR ALL PLAYERS FOR THE FIRST windowSize frames!
		for(int frame = 0; frame < simulRules.windowSize * simulRules.frameSkip; frame++)
		{
			Order dummy_order;
			dummy_order.plr_id = server_msg.keyState;
			cerr << "dummy order plrid: " << dummy_order.plr_id << endl;
			
			dummy_order.frameID = frame + simulRules.currentFrame;
			UnitInput.push_back(dummy_order);
		}
		
		sort(UnitInput.begin(), UnitInput.end());
		
		// if i'm the host, request a player name for this hero.
		if(state == "host")
		{
			serverSendRequestPlayerNameMessage(server_msg.keyState);
		}
		
	}
	else if(server_msg.serverCommand == 2) // "set playerID" message
	{
		myID = server_msg.keyState; // trololol. nice place to store the info.
		view.pushMessage("got playerID!");
		
		if(world.units.find(myID) != world.units.end())
		{
			cerr << "Binding camera to player " << myID << "\n";
			view.bindCamera(&world.units[myID]);
		}
		else
		{
			cerr << "Failed to bind camera! :(" << endl;
		}
	}
	else
	{
		cerr << "SERVERMESSAGEFUCK: " << server_msg.serverCommand << endl;
	}
}




void Game::processClientMsgs()
{
	for(int i=0; i<static_cast<int>(clientOrders.orders.size()); i++)
	{
		stringstream ss(clientOrders.orders[i]);
		
		int order_type;
		ss >> order_type;
		
		if(order_type == 1) // ordinary input order
		{
			Order tmp_order;
			ss >> tmp_order.plr_id;
			ss >> tmp_order.frameID;
			ss >> tmp_order.keyState;
			ss >> tmp_order.mousex >> tmp_order.mousey;
			ss >> tmp_order.mouseButton;
			
			UnitInput.push_back(tmp_order);
		}
		else if(order_type == 3) // chat message
		{
			int plrID;
			string line;
			
			ss >> plrID;
			getline(ss, line);
			
			stringstream chatMsg;
			chatMsg << "<" << Players[plrID].name << "> " << line;
			view.pushMessage(chatMsg.str());
		}
		
		else if(order_type == 2) // playerInfo message
		{
			cerr << "Got playerInfo message!" << endl;
			int plrID;
			string name;
			ss >> plrID >> name;
			Players[plrID].name = name;
			cerr << plrID << " " << name << endl;
			
			stringstream ss_viewMsg;
			ss_viewMsg << Players[plrID].name << " has connected!" << endl;
			view.pushMessage(ss_viewMsg.str());
			
			// set unit's name to match the players
			if(world.units.find(plrID) == world.units.end())
				cerr << "GOT playerInfo MESSAGE TOO SOON :G WHAT HAPPENS NOW??" << endl;
			else
			{
				cerr << "Assigning player identity (name) to corresponding unit." << endl;
				world.units[plrID].name = Players[plrID].name;
			}
		}
		
		else if(order_type == -1) // A COMMAND message from GOD (server)
		{
			
			Order tmp_order;
			tmp_order.plr_id = -1;
			ss >> tmp_order.frameID;
			ss >> tmp_order.serverCommand;
			
			for(int i=0; i<UnitInput.size(); i++)
				cerr << "(" << UnitInput[i].frameID << ", " << UnitInput[i].plr_id << ")" << endl;
			
			cerr << "GOT A SERVER MESSAGE: " << tmp_order.serverCommand << " " << tmp_order.frameID << "(current frame: " << simulRules.currentFrame << ", allowed frame: " << simulRules.allowedFrame << ")" << endl;
			
			ss >> tmp_order.keyState;
			
			UnitInput.push_back(tmp_order);
		}
		
		
		else if(order_type == -2) // instant reaction message from GOD
		{
			string cmd;
			ss >> cmd;
			if(cmd == "GO")
			{
				cerr << "Setting client state to 1" << endl;
				client_state = 1;
				fps_world.reset();
				fps_world.setStartTime( SDL_GetTicks() );
			}
			else if(cmd == "ALLOW")
			{
				int frame;
				ss >> frame;
				
				// cerr << "SERVER ALLOWED SIMULATION UP TO FRAME: " << frame << endl;
				simulRules.allowedFrame = frame;
			}
			else if(cmd == "GIVE_NAME") // request player name message
			{
				cerr << "Server wants to know my identity now. Sending: " << localPlayer.name << endl;
				stringstream ss;
				ss << "2 " << myID << " " << localPlayer.name << "#";
				clientSocket.write(ss.str());
			}
			else if(cmd == "UNIT") // unit copy message
			{
				cerr << "Creating a new unit as per instructions" << endl;
				int unitID;
				ss >> unitID;
				world.addUnit(unitID);
				world.units[unitID].handleCopyOrder(ss);
			}
			else if(cmd == "PROJECTILE")
			{
				int id; ss >> id;
				Location paska;
				world.addProjectile(paska, id);
				world.projectiles[id].handleCopyOrder(ss);
			}
			else if(cmd == "NEXT_UNIT_ID")
			{
				ss >> world._unitID_next_unit;
			}
			else if(cmd == "SIMUL")
			{
				cerr << "Set simulRules to instructed state" << endl;
				ss >> simulRules.currentFrame >> simulRules.windowSize >> simulRules.frameSkip >> simulRules.numPlayers >> simulRules.allowedFrame;
				cerr << simulRules.currentFrame << " " << simulRules.windowSize << " " << simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << endl;
			}
			else if(cmd == "CLIENT_STATE")
			{
				ss >> client_state;
				fps_world.reset();
				fps_world.setStartTime( SDL_GetTicks() );
			}
			else
			{
				cerr << "INSTANT ORDER FUCK!" << endl;
			}
			
		}
		
		else if(order_type == -4) // copy of an existing order
		{
			cerr << "Got a copy of an old message" << endl;
			Order tmp_order;
			ss >> tmp_order.frameID >> tmp_order.plr_id >> tmp_order.keyState >> tmp_order.mousex >> tmp_order.mousey >> tmp_order.serverCommand >> tmp_order.mouseButton;
			UnitInput.push_back(tmp_order);
		}
		else
		{
			cerr << "HOLY FUCK! I DONT UNDERSTAND SHIT :G" << endl;
		}
	}
	
	clientOrders.orders.clear();
}


void Game::client_tick()
{

	// check if we have new msgs from the server.
	if(clientSocket.readyToRead() == 1)
	{
		string msg = clientSocket.read();
		
		if(msg.size() == 0)
		{
			clientSocket.closeConnection();
			cerr << "Client connection has died. :(" << endl;
			state = "menu";
		}
		
		clientOrders.insert(msg); // give it to orderhandler to be parsed down to single commands
		processClientMsgs();
	}
	
	
	string key = userio.getSingleKey();

	camera_handling();
	
	if(key.size() != 0)
	{
		if(key == "return")
			client_state ^= 2;
		if(key == "f11")
			view.toggleFullscreen();
		if(key == "f10")
			view.toggleLightingStatus();
		if(key == "p")
		{
			if(state == "host")
			{
				// send spawn monster message
				serverSendMonsterSpawn();
			}
		}
		if(key == "f9")
			world.show_errors ^= 1;
		
		if(client_state & 2)
		{
			string nick;
			nick.append("<");
			nick.append(Players[myID].name);
			nick.append("> ");
			
			if(key.size() == 1)
				clientCommand.append(key);
			
			if(key == "backspace" && clientCommand.size() > 0)
				clientCommand.resize(clientCommand.size()-1);

			if(key == "escape") {
				client_state ^= 2;
				clientCommand = "";
				nick = "";
			}
			
			if(key == "space")
				clientCommand.append(" ");
			nick.append(clientCommand);
			view.setCurrentClientCommand(nick);
		}
		else
		{
			
			if(key == "return") {
				// handle client command
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
				cerr << "User pressed ESC, shutting down." << endl;
				SDL_Quit();
				exit(0);
			}
			
			if(key == "g") {
				if (client_state & 4) {
					SDL_WM_GrabInput(SDL_GRAB_OFF);
					SDL_ShowCursor(1);
				} else {
					SDL_WM_GrabInput(SDL_GRAB_ON);
					SDL_ShowCursor(0);
				}
				client_state ^= 4;
			}
		}
	}
	
	// if state_descriptor == 0, the userIO
	// is used by HOST functions. Do not interfere.
	if( ((state == "client") || (state_descriptor != 0)) && (client_state & 1))  
	{

		view.setZombiesLeft(world.getZombies());
		update_kills();
		view.setLocalPlayerKills(Players[myID].kills);

		// this is acceptable because the size is guaranteed to be insignificantly small
		sort(UnitInput.begin(), UnitInput.end());
		
		// deliver any world message events to graphics structure, and erase them from world data.
		for(int i=0; i<world.worldMessages.size(); i++)
			view.pushMessage(world.worldMessages[i]);
		world.worldMessages.clear();
		
		// handle any world events <-> graphics structure
		for(int i=0; i<world.events.size(); i++)
		{
			WorldEvent& event = world.events[i];
			if(event.type == World::DAMAGE_BULLET)
				view.genParticles(event.position, event.velocity, 4, 0.3, 0.4f, 0.6f, 0.2f, 0.2f);
			else if(event.type == World::DAMAGE_DEVOUR)
				view.genParticles(event.position, event.velocity, 9, 0.7, 0.4f, 0.9f, 0.2f, 0.2f);
			else if(event.type == World::DEATH_ENEMY)
				view.genParticles(event.position, event.velocity, 30, 2.0, 1.0f, 0.1f, 0.5f, 0.2f);
			else if(event.type == World::DEATH_PLAYER)
				view.genParticles(event.position, event.velocity, 30, 2.0, 1.0f, 1.0f, 0.2f, 0.2f);
			else
				cerr << "UNKOWN WORLD EVENT OCCURRED" << endl;
		}
		world.events.clear();
		
		
		if(myID != -1)
		{
			view.setLocalPlayerName(Players[myID].name);
			view.setLocalPlayerHP(world.units[myID].hitpoints);
		}
		
		
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
			
			// run simulation for one WorldFrame
			world.worldTick(simulRules.currentFrame);
			simulRules.currentFrame++;
		}
	}
	
}

void Game::update_kills() {
	while(!world.kills.empty()) {
		Players[world.kills.back()].kills++;
		world.kills.pop_back();
	}
}

void Game::camera_handling()
{
	int wheel_status = userio.getMouseWheelScrolled();
	if (wheel_status == 1)
		view.mouseUp();
	if (wheel_status == 2)
		view.mouseDown();
}
