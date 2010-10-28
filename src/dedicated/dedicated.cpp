
#include "dedicated.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <sys/time.h>

using namespace std;

long long time_now()
{
	timeval t;
	gettimeofday(&t,NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}


DedicatedServer::DedicatedServer(): fps_world(0)
{
	pause_state = WAITING_PLAYERS;
	serverAllow = 0;
	init();
}

void DedicatedServer::init()
{
	long long milliseconds = time_now();
	fps_world.reset(milliseconds);
	
	srand(time(0));
}

void DedicatedServer::send_to_all(const std::string& msg)
{
	for(auto it = Players.begin(); it != Players.end(); ++it)
	{
		// dont write to disconnected players
		if(sockets.alive(it->first) && it->second.connectionState == 1)
		{
			sockets.write(it->first, msg);
		}
	}
}

bool DedicatedServer::start(int port)
{
	return sockets.listen(port);
}

void DedicatedServer::disconnect(int leaver)
{
	stringstream discCommand;
	discCommand << -1 << " " << (Players[leaver].last_order + 1) << " 100 " << leaver << "#";
	serverMsgs.push_back( discCommand.str() );
	
	cerr << "Saving disconnecting character with key: " << Players[leaver].key << endl;
	dormantPlayers[Players[leaver].key] = Players[leaver];
	Players.erase(leaver);
	
	sockets.close(leaver);
}

void DedicatedServer::check_messages_from_clients()
{
	// if there are leavers, send a kill order against one of them
	int leaver = -1;
	
	// mirror any client commands to all clients
	for(auto i = Players.begin(); i != Players.end(); ++i)
	{
		if(!sockets.alive(i->first))
		{
//			cerr << "LEAVER: " << i->first << ", lastOrder: " << i->second.last_order << endl;
			leaver = i->first;
			continue;
		}
		
		
		vector<string>& msgs = sockets.read(i->first);
		int conn_state = Players[i->first].connectionState;
		
		if(conn_state == 0)
		{
			for(size_t k=0; k < msgs.size(); k++)
				handleSignInMessage(i->first, msgs[k]);
			msgs.clear();
		}
		else if(conn_state == 1)
		{
			for(size_t k=0; k < msgs.size(); k++)
			{
				parseClientMsg(msgs[k], i->first, i->second);
			}
			msgs.clear();
		}
	}
	
	// this message will go to local messagehandling at the end of function
	if(leaver != -1) // there is a leaver!!
	{
		disconnect(leaver);
	}
}

void DedicatedServer::processServerMsgs()
{
	// this is acceptable because the size is guaranteed to be insignificantly small
	sort(UnitInput.begin(), UnitInput.end());
	
	// handle any server commands intended for this frame
	while(!UnitInput.empty() && (UnitInput.back().plr_id == SERVER_ID) && (UnitInput.back().frameID == simulRules.currentFrame))
	{
		Order server_command = UnitInput.back();
		UnitInput.pop_back();
		ServerHandleServerMessage(server_command);
	}
}

void DedicatedServer::host_tick()
{
	// Read and write network data.
	sockets.tick();

	acceptConnections();

	check_messages_from_clients();
	
	unsigned minAllowed = UINT_MAX;
	for(auto it = Players.begin(); it != Players.end(); ++it)
	{
		if(it->second.connectionState == 1 && it->second.last_order < minAllowed)
			minAllowed = it->second.last_order;
	}
	
	if(minAllowed < simulRules.windowSize)
		minAllowed = simulRules.windowSize;
	
	if( (minAllowed != UINT_MAX) )
		simulRules.allowedFrame = minAllowed;
	
	if( (minAllowed != UINT_MAX) && (minAllowed > simulRules.currentFrame) )
		minAllowed = simulRules.currentFrame;
	
	if( pause_state == RUNNING && (minAllowed != UINT_MAX) && (minAllowed > serverAllow) )
	{
		stringstream allowSimulation_msg;
		allowSimulation_msg << "-2 ALLOW " << minAllowed << "#";
		
		serverAllow = minAllowed;
		serverMsgs.push_back(allowSimulation_msg.str());
	}
	
	// transmit serverMsgs to players
	for(size_t k = 0; k < serverMsgs.size(); ++k)
	{
		// give msg to local handling and send to others.
		clientOrders.insert(serverMsgs[k]);
		
		send_to_all(serverMsgs[k]);
	}
	serverMsgs.clear();
	
	
	if(pause_state != RUNNING)
	{
		if(simulRules.numPlayers > 0 && pause_state == WAITING_PLAYERS)
		{
			// send the beginning commands to all players currently connected.
			cerr << " I HAVE : " << simulRules.numPlayers << " PLAYERS " << endl;
			
			pause_state = RUNNING;
			serverMsgs.push_back("-2 GO#");
		}
		else
		{
			return;
		}
	}
	
	// prepare to update game world.
	processClientMsgs();
	
	// THIS DOESNT ACTUALLY WORK (I THINK). THE ONLY REASON THE SERVER WORKS, IS THAT THIS CODE IS _NEVER_ EXECUTED
	// the level can kind of shut down when there's no one there.
	if( Players.empty() && UnitInput.empty() )
	{
		simulRules.reset();
		
		long long milliseconds = time_now();
		fps_world.reset(milliseconds);
		
		cerr << "World shutting down." << endl;
		pause_state = WAITING_PLAYERS;
		return;
	}
	
	processServerMsgs();
	
	long long milliseconds = time_now();
	if( (simulRules.currentFrame < simulRules.allowedFrame) && fps_world.need_to_draw(milliseconds) )
	{
		simulateWorldFrame();
	}
	else
	{
//		cerr << "No need to simulate yet!\n";
	}
}

void DedicatedServer::simulateWorldFrame()
{
	if( (UnitInput.back().plr_id == SERVER_ID) && (UnitInput.back().frameID != simulRules.currentFrame) )
		cerr << "ERROR: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;
	
	fps_world.insert();
	
	
	static unsigned long checksums[10] = { 0 };
	int current = simulRules.currentFrame % 10;
	checksums[current] = world.checksum();

	while(!UnitInput.empty() && UnitInput.back().frameID == simulRules.currentFrame)
	{
		Order tmp = UnitInput.back();
		UnitInput.pop_back();
		
		if(tmp.plr_id == SERVER_ID)
		{
			cerr << "MOTHERFUCKER FUCKING FUCK YOU MAN?= JUST FUCK YOU!!" << endl;
			break;
		}

		unsigned long cs_tmp = checksums[(tmp.frameID + 10 - 5) % 10];
		if((simulRules.currentFrame > 10) && (tmp.checksum != cs_tmp) && (tmp.checksum != 0))
		{
			std::cerr << "OOS, client: " << tmp.frameID << ", server: " << simulRules.currentFrame << std::endl;
			std::cerr << tmp.checksum << std::endl;
			std::cerr << checksums[(tmp.frameID + 10 - 5) % 10] << std::endl;

			cerr << "server side:" << std::endl;
			for (auto it = world.units.begin(); it != world.units.end(); ++it)
			{
				int id = it->first;
				Location pos = it->second.position;
				cerr << id << ": " << pos << std::endl;
			}
			pause_state = PAUSED;
		}
		
		world.units[tmp.plr_id].updateInput(tmp.keyState, tmp.mousex, tmp.mousey, tmp.mouseButtons);
	}
	
	if(!UnitInput.empty() && UnitInput.back().frameID < simulRules.currentFrame)
	{
		cerr << "Server has somehow skipped an order :(" << endl;
	}
	
	world.worldTick(simulRules.currentFrame);
	simulRules.currentFrame++;
	
	
	handleWorldEvents();
}

void DedicatedServer::parseClientMsg(const std::string& msg, int player_id, PlayerInfo& player)
{
	string new_message = msg + "#";
	// keep track of last frames for which orders have been received.
	stringstream ss(new_message);
	string orderWord;
	ss >> orderWord;
	
	if(orderWord == "1")
	{
		int order_player_id, frame;
		ss >> order_player_id >> frame;
		player.last_order = frame;
	}
	else if(orderWord == "2") // someone is sending an introduction message. I, as the GOD, should rewrite some of that message :D
	{
		int id;
		string introductionName;
		
		ss >> id;
		ss.ignore();
		ss >> introductionName;
		
		if(id == player_id)
		{
			// everything checks out.
			stringstream new_message_ss;
			
			if(player.name == "")
				player.name = introductionName;
			
			// create a better message and distribute that to players.
			new_message_ss << "2 " << id << " " << Players[id].kills << " " << Players[id].deaths << " " << Players[id].name << "#";

			new_message = new_message_ss.str();
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
			return;
		}
		
		if(cmd == "SWARM#")
		{
			serverSendMonsterSpawn(70);
			
			stringstream chatmsg;
			chatmsg << "3 -1 ^r" << player.name << "^w has performed a dark ritual.. the ^rswarm ^wis anigh..#";
			serverMsgs.push_back(chatmsg.str());
			
			return;
		}
	}
	else if(orderWord == "-1")
	{
		// WTF?? Someone is trying to impersonate GOD (that's me). Maybe I should lay some thunder on his ass?
		cerr << "SOME IS IMPERSONATING GOD!! :" << msg << endl;
		new_message = "";
		
		// disconnect the fucker
		sockets.close(player_id);
		
		stringstream chatmsg;
		chatmsg << "3 -1 ^r" << player.name << " ^w has attempted to impersonate ^GME ^w .. --> ^Rdisconnected#";
		serverMsgs.push_back(chatmsg.str());
	}
	else
	{
		cerr << "BAD CLIENTMESSAGE: '" << msg << "'\n";
	}
	
	// commands passed down to local message handling before sending to others.
	clientOrders.insert(new_message);
	send_to_all(new_message);
}


void DedicatedServer::acceptConnections()
{
	static int id = world.nextPlayerID();
	if(sockets.accept(id))
	{
		cerr << "looks like someone is connecting :O" << endl;
		Players[id].connectionState = 0;
		id = world.nextPlayerID();
	}
}

// server messages read from the network
void DedicatedServer::ServerHandleServerMessage(const Order& server_msg)
{
	if(server_msg.serverCommand == 3) // pause!
	{
		pause_state = PAUSED;
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
		
		cerr << "Adding a new hero at frame " << simulRules.currentFrame << ", units.size() = " << world.units.size() << endl;
		cerr << "Creating dummy input for new hero." << endl;
		
		// WE MUST CREATE DUMMY INPUT FOR ALL PLAYERS FOR THE FIRST windowSize frames!
		for(unsigned frame = 0; frame < simulRules.windowSize * simulRules.frameSkip; ++frame)
		{
			Order dummy_order;
			dummy_order.plr_id = server_msg.keyState;
			dummy_order.checksum = 0;
			cerr << "dummy order plrid: " << dummy_order.plr_id << endl;
			
			dummy_order.frameID = frame + simulRules.currentFrame;
			UnitInput.push_back(dummy_order);
		}
		
		sort(UnitInput.begin(), UnitInput.end());
		serverSendRequestPlayerNameMessage(server_msg.keyState);
	}
	else if(server_msg.serverCommand == 2) // "set playerID" message
	{
		cerr << "SERVER HAS NO ID, CAN NOT CHANGE IT" << endl;
	}
	else
	{
		cerr << "SERVERMESSAGEFUCK: " << server_msg.serverCommand << endl;
	}
}

void DedicatedServer::processClientMsgs()
{
	for(size_t i = 0; i < clientOrders.orders.size(); ++i)
	{
		
		if(clientOrders.orders[i] == "")
		{
			cerr << "SERVER: I encountered an empty order :o how peculiar :D" << endl;
			continue;
		}

		processClientMsg(clientOrders.orders[i]);
		
	}
	
	clientOrders.orders.clear();
}

void DedicatedServer::processClientMsg(const std::string& msg)
{
	stringstream ss(msg);
	
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
		ss >> tmp_order.checksum;

		UnitInput.push_back(tmp_order);
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
			cerr << "Setting pause state to RUNNING" << endl;
			pause_state = RUNNING;
			
			long long milliseconds = time_now();
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
			int nopause = 0;
			ss >> nopause;
			pause_state = (nopause ? RUNNING : PAUSED);
			
			long long milliseconds = time_now();
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


void DedicatedServer::handleWorldEvents()
{
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

