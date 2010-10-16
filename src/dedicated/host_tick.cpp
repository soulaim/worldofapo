
#include "dedicated.h"

#include <algorithm>

#include <sys/time.h>

using namespace std;


void DedicatedServer::host_tick()
{
	// accept any incoming connections
	
	/*
	timeval time;
	gettimeofday(&time,NULL);
	long long time_ms = time.tv_sec * 1000 + time.tv_usec / 1000;
	
	cerr << time_ms << endl;
	*/
	
	if(serverSocket.readyToRead() == 1)
		acceptConnections();
	
	// if there's any data to be read from clients, then read it
	sockets.get_readable();
	sockets.read_selected();
	
	
	/*
	// MAYBE THIS DOESNT NEED TO BE DONE SO FUCKING OFTEN
	timeval time;
	gettimeofday(&time,NULL);
	long long time_ms = time.tv_sec * 1000 + time.tv_usec / 1000;
	if(simulRules.numPlayers == 0)
		fps_world.reset(time_ms);
	*/
	
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
		
		
		int conn_state = Players[i->first].connectionState;
		
		if(conn_state == 0)
		{
			for(size_t k=0; k < i->second.msgs.size(); k++)
				handleSignInMessage(i->first, i->second.msgs[k]);
			i->second.msgs.clear();
		}
		else if(conn_state == 1)
		{
			for(size_t k=0; k < i->second.msgs.size(); k++)
			{
				i->second.msgs[k].append("#");
				
				
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
				else if(orderWord == "2") // someone is sending an introduction message. I, as the GOD, should rewrite some of that message :D
				{
					int id;
					string introductionName;
					
					ss >> id;
					ss.ignore();
					ss >> introductionName;
					
					
					if(id == i->first)
					{
						// everything checks out.
						stringstream new_message;
						
						if(Players[id].name == "")
							Players[id].name = introductionName;
						
						// create a better message and distribute that to players.
						new_message << "2 " << id << " " << Players[id].kills << " " << Players[id].deaths << " " << Players[id].name << "#";
						i->second.msgs[k] = new_message.str();
					}
				}
				else if(orderWord == "3") // chat message!
				{
					string id;
					string cmd;
					
					ss >> id >> cmd;
					
					cerr << "Chat <" << id << "> " << cmd << endl;
					
					if(cmd == "CRTZOMB#")
					{
						cerr << "SPAWNING MONSTER" << endl;
						serverSendMonsterSpawn();
						continue;
					}
					
					if(cmd == "SWARM#")
					{
						serverSendMonsterSpawn(70);
						
						stringstream chatmsg;
						chatmsg << "3 -1 ^r" << Players[i->first].name << "^w has performed a dark ritual.. the ^rswarm ^wis anigh..#";
						serverMsgs.push_back(chatmsg.str());
						
						continue;
					}
				}
				else if(orderWord == "-1")
				{
					// WTF?? Someone is trying to impersonate GOD (that's me). Maybe I should lay some thunder on his ass?
					cerr << "SOME IS IMPERSONATING GOD!! :" << i->second.msgs[k] << endl;
					i->second.msgs[k] = "";
					
					// disconnect the fucker
					i->second.alive = false;
					
					stringstream chatmsg;
					chatmsg << "3 -1 ^r" << Players[i->first].name << " ^w has attempted to impersonate ^GME ^w .. --> ^Rdisconnected#";
					serverMsgs.push_back(chatmsg.str());
				}
				
				
				// commands passed down to local message handling before sending to others.
				clientOrders.insert(i->second.msgs[k]);
				
				
				
				for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
				{
					// dont write to disconnected players
					if(target->second.alive && (Players[target->first].connectionState == 1))
						target->second.write(i->second.msgs[k]);
				}
				
			}
			i->second.msgs.clear();
		}
	}
	
	// this message will go to local messagehandling at the end of function
	if(leaver != -1) // there is a leaver!!
	{
		stringstream discCommand;
		discCommand << -1 << " " << (sockets.sockets[leaver].last_order + 1) << " 100 " << leaver << "#";
		serverMsgs.push_back( discCommand.str() );
		
		
		cerr << "Saving disconnecting character with key: " << Players[leaver].key << endl;
		dormantPlayers[Players[leaver].key] = Players[leaver];
		Players.erase(leaver);
		
		sockets.erase_id(leaver);
	}
	
	unsigned minAllowed = UINT_MAX;
	for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
	{
		if(Players[target->first].connectionState == 1 && target->second.last_order < minAllowed)
			minAllowed = target->second.last_order;
	}
	
	if(minAllowed < simulRules.windowSize)
		minAllowed = simulRules.windowSize;
	
	if( (minAllowed != UINT_MAX) )
		simulRules.allowedFrame = minAllowed;
	
	if( (minAllowed != UINT_MAX) && (minAllowed > simulRules.currentFrame) )
		minAllowed = simulRules.currentFrame;
	
	if( (minAllowed != UINT_MAX) && (minAllowed > serverAllow) )
	{
		stringstream allowSimulation_msg;
		allowSimulation_msg << "-2 ALLOW " << minAllowed << "#";
		
		serverAllow = minAllowed;
		serverMsgs.push_back(allowSimulation_msg.str());
	}
	
	// transmit serverMsgs to players
	for(int k=0; k < static_cast<int>(serverMsgs.size()); k++)
	{
		// give msg to local handling and send to others.
		clientOrders.insert(serverMsgs[k]);
		
		for(map<int, MU_Socket>::iterator i = sockets.sockets.begin(); i != sockets.sockets.end(); i++)
		{
			if(Players[i->first].connectionState == 1)
				i->second.write(serverMsgs[k]);
		}
	}
	serverMsgs.clear();
	
	
	if(state_descriptor == 0)
	{
		if(simulRules.numPlayers > 0)
		{
			// send the beginning commands to all players currently connected.
			cerr << " I HAVE : " << simulRules.numPlayers << " PLAYERS " << endl;
			
			state_descriptor = 1;
			serverMsgs.push_back("-2 GO#");
		}
		else
		{
			/*
			// update tick counts anyway..
			timeval t;
			gettimeofday(&t,NULL);
			long long milliseconds = t.tv_sec * 1000 + t.tv_usec / 1000;
			if(fps_world.need_to_draw(milliseconds))
				fps_world.insert();
			*/
			return;
		}
	}
	
	// prepare to update game world.
	ServerProcessClientMsgs();
	
	// this is acceptable because the size is guaranteed to be insignificantly small
	sort(UnitInput.begin(), UnitInput.end());
	
	
	// THIS DOESNT ACTUALLY WORK (I THINK). THE ONLY REASON THE SERVER WORKS, IS THAT THIS CODE IS _NEVER_ EXECUTED
	// the level can kind of shut down when there's no one there.
	if( (sockets.sockets.size() == 0) && (UnitInput.size() == 0) )
	{
		UnitInput.clear(); // redundant
		simulRules.reset();
		
		timeval t;
		gettimeofday(&t,NULL);
		long long milliseconds = t.tv_sec * 1000 + t.tv_usec / 1000;
		fps_world.reset(milliseconds);
		
		cerr << "World shutting down." << endl;
		state_descriptor = 0;
		return;
	}
	
	// handle any server commands intended for this frame
	while((UnitInput.back().plr_id == -1) && (UnitInput.back().frameID == simulRules.currentFrame))
	{
		Order server_command = UnitInput.back();
		UnitInput.pop_back();
		ServerHandleServerMessage(server_command);
	}
	
	
	
	timeval t;
	gettimeofday(&t,NULL);
	long long milliseconds = t.tv_sec * 1000 + t.tv_usec / 1000;
	
	//																									// this seems like THE wrong way to do it.
	if( (simulRules.currentFrame < simulRules.allowedFrame) && fps_world.need_to_draw(milliseconds) ) //|| (sockets.sockets.size() == 0) )
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
		
	}
}





void DedicatedServer::acceptConnections()
{
	
	cerr << "looks like someone is connecting :O" << endl;
	
	int playerID_val = world.nextPlayerID();
	serverSocket.accept_connection(sockets, playerID_val);
	Players[playerID_val].connectionState = 0;
}










// server messages read from the network
void DedicatedServer::ServerHandleServerMessage(const Order& server_msg)
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
		simulRules.numPlayers--;
		// BWAHAHAHA...
	}
	
	else if(server_msg.serverCommand == 1) // ADDHERO message
	{
		world.addUnit(server_msg.keyState);
		
		// just to be sure.
		world.units[server_msg.keyState].name = Players[server_msg.keyState].name;
		
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
void DedicatedServer::ServerProcessClientMsgs()
{
	for(size_t i = 0; i < clientOrders.orders.size(); ++i)
	{
		
		if(clientOrders.orders[i] == "")
		{
			cerr << "SERVER: I encountered an empty order :o how peculiar :D" << endl;
			continue;
		}
		
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
		}
		
		else if(order_type == 2) // playerInfo message
		{
			cerr << "SERVER Got playerInfo message!" << endl;
			int plrID, kills, deaths;
			string name;
			ss >> plrID >> kills >> deaths;
			
			ss.ignore(); // eat away the extra space delimiter
			getline(ss, name);
			
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
				
				timeval t;
				gettimeofday(&t,NULL);
				long long milliseconds = t.tv_sec * 1000 + t.tv_usec / 1000;
				fps_world.reset(milliseconds);
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
				
				timeval t;
				gettimeofday(&t,NULL);
				long long milliseconds = t.tv_sec * 1000 + t.tv_usec / 1000;
				fps_world.reset(milliseconds);
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
