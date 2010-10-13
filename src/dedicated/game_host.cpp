
#include "../game.h"

#include <algorithm>

using namespace std;

void Game::serverSendMonsterSpawn()
{
	stringstream tmp_msg;
	tmp_msg << "-1 " << (serverAllow+1) << " 10" << "#";
	for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
		target->second.write(tmp_msg.str());
}

void Game::serverSendRequestPlayerNameMessage(int player_id)
{
	cerr << "Sending a request to the new player to identify himself!" << endl;
	sockets.sockets[player_id].write("-2 GIVE_NAME#");
}

void Game::host_tick()
{
	acceptConnections();
	
	// if there's any data to be read from clients, then read it
	sockets.get_readable();
	sockets.read_selected();
	
	// if there are leavers, send a kill order against one of them
	int leaver = -1;
	
	// mirror any client commands to all clients
	for(map<int, MU_Socket>::iterator i = sockets.sockets.begin(); i != sockets.sockets.end(); i++)
	{
		if(!i->second.alive)
		{
			cerr << "LEAVER: " << i->first << ", lastOrder: " << i->second.last_order << endl;
			leaver = i->first;
			continue;
		}
		
		for(int k=0; k<static_cast<int>(i->second.msgs.size()); k++)
		{
			i->second.msgs[k].append("#");
			
			// commands passed down to local message handling before sending to others.
			clientOrders.insert(i->second.msgs[k]);
			
			for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
			{
				// dont write to disconnected players
				if(target->second.alive)
					target->second.write(i->second.msgs[k]);
			}
			
			// keep track of last frames for which orders have been received.
			stringstream ss(i->second.msgs[k]);
			string orderWord;
			ss >> orderWord;
			
			if(orderWord == "1")
			{
				int order_player_id, frame;
				ss >> order_player_id >> frame;
				i->second.last_order = frame;
			}
		}
		i->second.msgs.clear();
	}
	
	// this message will go to local messagehandling at the end of function
	if(leaver != -1) // there is a leaver!!
	{
		stringstream discCommand;
		discCommand << -1 << " " << (sockets.sockets[leaver].last_order + 1) << " 100 " << leaver << "#";
		serverMsgs.push_back( discCommand.str() );
		
		sockets.erase_id(leaver);
	}
	
	unsigned minAllowed = UINT_MAX;
	for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
	{
		if(target->second.last_order < minAllowed)
			minAllowed = target->second.last_order;
	}
	
	if(minAllowed < simulRules.windowSize)
		minAllowed = simulRules.windowSize;
	
	
	if( (minAllowed != UINT_MAX) && (minAllowed > serverAllow) )
	{
		stringstream allowSimulation_msg;
		allowSimulation_msg << "-2 ALLOW " << minAllowed << "#";
		
		serverAllow = minAllowed;
		serverMsgs.push_back(allowSimulation_msg.str());
	}
	
	if((state_descriptor == 0) && (sockets.sockets.size() > 0))
	{
		state_descriptor = 1;
		
		// send the beginning commands to all players currently connected.
		serverMsgs.push_back("-2 GO#");
	}
	
	
	
	// transmit serverMsgs to players
	for(int k=0; k < static_cast<int>(serverMsgs.size()); k++)
	{
		// give msg to local handling and send to others.
		clientOrders.insert(serverMsgs[k]);
		
		for(map<int, MU_Socket>::iterator i = sockets.sockets.begin(); i != sockets.sockets.end(); i++)
		{
			i->second.write(serverMsgs[k]);
		}
	}
	serverMsgs.clear();
	
	
	// prepare to update game world.
	ServerProcessClientMsgs();
	
	
	
	// this is acceptable because the size is guaranteed to be insignificantly small
	sort(UnitInput.begin(), UnitInput.end());

	
	// the level can kind of shut down when there's no one there.
	if( (sockets.sockets.size() == 0) && (UnitInput.size() == 0) )
	{
		UnitInput.clear(); // redundant
		simulRules.reset();
		return;
	}
	
	// handle any server commands intended for this frame
	while((UnitInput.back().plr_id == -1) && (UnitInput.back().frameID == simulRules.currentFrame))
	{
		Order server_command = UnitInput.back();
		UnitInput.pop_back();
		ServerHandleServerMessage(server_command);
	}
	
	
	if( (simulRules.currentFrame < simulRules.allowedFrame) )
	{
		if( (UnitInput.back().plr_id == -1) && (UnitInput.back().frameID != simulRules.currentFrame) )
			cerr << "ERROR: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;
		
		fps_world.insert();
		
		
		while(!UnitInput.empty() && UnitInput.back().frameID == simulRules.currentFrame)
		{
			Order tmp = UnitInput.back();
			UnitInput.pop_back();
			
			if(tmp.plr_id == -1)
			{
				cerr << "MOTHERFUCKER FUCKING FUCK YOU MAN?= JUST FUCK YOU!!" << endl;
				break;
			}
			
			world.units[tmp.plr_id].updateInput(tmp.keyState, tmp.mousex, tmp.mousey, tmp.mouseButtons);
		}
		
		if(!UnitInput.empty() && UnitInput.back().frameID < simulRules.currentFrame)
		{
			cerr << "Server has somehow skipped an order :(" << endl;
		}
		
		
		// run simulation for one WorldFrame
		world.worldTick(simulRules.currentFrame);
		simulRules.currentFrame++;
		
		
		
		
		
		
		// output events to show the server is still in sync.
		for(size_t i = 0; i < world.events.size(); ++i)
		{
			WorldEvent& event = world.events[i];
			
			if(event.type == World::DEATH_ENEMY)
			{
				if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
				{
					Players[event.actor_id].kills++;
					cerr << Players[event.actor_id].name << " has killed" << endl;
				}
			}
			
			if(event.type == World::DEATH_PLAYER)
			{
				if( (world.units.find(event.actor_id) != world.units.end()) && world.units[event.actor_id].human())
				{
					Players[event.actor_id].kills++;
					cerr << Players[event.actor_id].name << " has killed" << endl;
				}
				
				if( (world.units.find(event.target_id) != world.units.end()) && world.units[event.target_id].human())
				{
					Players[event.target_id].deaths++;
					cerr << Players[event.target_id].name << " has died" << endl;
				}
			}
		}
		
		world.events.clear();
		
		
		
		
		
		// sounds lol?
		/*
		for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
			playSound(iter->second.soundInfo, iter->second.position);
		*/
	}
}





void Game::acceptConnections()
{
	
	// accept any incoming connections
	if(serverSocket.readyToRead() == 1)
	{
		
		cerr << "looks like someone is connecting :O" << endl;
		
		int playerID_val = world.nextPlayerID();
		serverSocket.accept_connection(sockets, playerID_val);
		
		// if game in progress, inform everyone else of a new connecting player TODO
		MU_Socket& connectingPlayer = sockets.sockets[playerID_val];
		
		
		// send new player the current state of the world:
		for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
			connectingPlayer.write(iter->second.copyOrder(iter->first));
		
		// send new player the current state of the world:
		for(map<int, Projectile>::iterator iter = world.projectiles.begin(); iter != world.projectiles.end(); iter++)
			connectingPlayer.write(iter->second.copyOrder(iter->first));
		
		// send new player current pending orders
		for(size_t i = 0; i < UnitInput.size(); ++i)
			connectingPlayer.write(UnitInput[i].copyOrder());
		
		// tell the new player what his player ID is.
		stringstream playerID_msg;
		
		playerID_msg << "-1 " << (simulRules.currentFrame + simulRules.windowSize) << " 2 " << playerID_val << "#";
		connectingPlayer.write(playerID_msg.str());
		cerr << "player is expected to assume his role at frame " << (simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize) << endl;
		
		// send the new player some generic info about other players
		for(map<int, PlayerInfo>::iterator iter = Players.begin(); iter != Players.end(); iter++)
		{
			cerr << "sending info!" << endl;
			stringstream playerInfo_msg;
			playerInfo_msg << "2 " << iter->first << " " << iter->second.name << " " << iter->second.kills << " " << iter->second.deaths << "#";
			connectingPlayer.write(playerInfo_msg.str());
		}

		// send to everyone the ADDHERO msg
		int birth_time = simulRules.currentFrame + simulRules.windowSize;
		
		stringstream createHero_msg;
		connectingPlayer.last_order = birth_time + simulRules.frameSkip * simulRules.windowSize;
		
		createHero_msg << "-1 " << birth_time << " 1 " << playerID_val << "#";
		serverMsgs.push_back(createHero_msg.str());
		cerr << "Hero for player " << playerID_val << " is scheduled for birth at frame " << birth_time << endl;
		
		stringstream nextUnit_msg;
		nextUnit_msg << "-2 NEXT_UNIT_ID " << world._unitID_next_unit << "#";
		connectingPlayer.write(nextUnit_msg.str());
		
		// Now that all game info has been sent, can send messages to allow the client to start his own simulation.
		stringstream simulRules_msg;
		simulRules_msg << "-2 SIMUL " << simulRules.currentFrame << " " << simulRules.windowSize << " " <<  simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << "#";
		connectingPlayer.write(simulRules_msg.str());
		
		// go!
		stringstream clientState_msg;
		clientState_msg << "-2 CLIENT_STATE " << client_state << "#";
		connectingPlayer.write(clientState_msg.str());
		
	}
}












// server messages read from the network
void Game::ServerHandleServerMessage(const Order& server_msg)
{
	if(server_msg.serverCommand == 3) // pause!
	{
		client_state = 0;
		cerr << "SERVER: Pausing the game at frame " << simulRules.currentFrame << endl;
	}
	
	else if(server_msg.serverCommand == 10)
	{
		world.addUnit(world.nextUnitID(), false);
	}
	
	else if(server_msg.serverCommand == 100) // SOME PLAYER HAS DISCONNECTED
	{
		cerr << "THIS IS WHAT SERVER SHOULD DO WHEN DISCONNECT HAPPENS" << endl;
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
		cerr << "Creating dummy input for new hero." << endl;
		
		// WE MUST CREATE DUMMY INPUT FOR ALL PLAYERS FOR THE FIRST windowSize frames!
		for(unsigned frame = 0; frame < simulRules.windowSize * simulRules.frameSkip; ++frame)
		{
			Order dummy_order;
			dummy_order.plr_id = server_msg.keyState;
			cerr << "dummy order plrid: " << dummy_order.plr_id << endl;
			
			dummy_order.frameID = frame + simulRules.currentFrame;
			UnitInput.push_back(dummy_order);
		}
		
		sort(UnitInput.begin(), UnitInput.end());
		serverSendRequestPlayerNameMessage(server_msg.keyState);
	}
	else if(server_msg.serverCommand == 2) // "set playerID" message
	{
		myID = server_msg.keyState; // trololol. nice place to store the info.
	}
	else
	{
		cerr << "SERVERMESSAGEFUCK: " << server_msg.serverCommand << endl;
	}
}



// client messages read from the network
void Game::ServerProcessClientMsgs()
{
	for(size_t i = 0; i < clientOrders.orders.size(); ++i)
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
			ss >> tmp_order.mouseButtons;
			
			UnitInput.push_back(tmp_order);
		}
		else if(order_type == 3) // chat message
		{
			cerr << "GOT Chat message" << endl;
		}
		
		else if(order_type == 2) // playerInfo message
		{
			cerr << "SERVER Got playerInfo message!" << endl;
			int plrID, kills, deaths;
			string name;
			ss >> plrID >> name >> kills >> deaths;
			Players[plrID].name = name;
			Players[plrID].kills = kills;
			Players[plrID].deaths = deaths;
			
			cerr << plrID << " " << name << " (" << kills << "/" << deaths << ")" << endl;
			
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
				cerr << "I AM THOR!" << endl;
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
				cerr << "WTF? Something asked the server to SET the SIMULRULES. NOT DOING IT." << endl;
				//cerr << "Set simulRules to instructed state" << endl;
				//ss >> simulRules.currentFrame >> simulRules.windowSize >> simulRules.frameSkip >> simulRules.numPlayers >> simulRules.allowedFrame;
				//cerr << simulRules.currentFrame << " " << simulRules.windowSize << " " << simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << endl;
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
			cerr << "Server got a copy of an old message??" << endl;
			
			//Order tmp_order;
			//ss >> tmp_order.frameID >> tmp_order.plr_id >> tmp_order.keyState >> tmp_order.mousex >> tmp_order.mousey >> tmp_order.serverCommand >> tmp_order.mouseButtons;
			//UnitInput.push_back(tmp_order);
		}
		else
		{
			cerr << "HOLY FUCK! I DONT UNDERSTAND SHIT :G" << endl;
		}
	}
	
	clientOrders.orders.clear();
}
