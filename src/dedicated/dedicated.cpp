#include "dedicated.h"
#include "../modelfactory.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>

#ifndef _WIN32
#include <sys/time.h>
#endif

#define SLEEP_IF_POSSIBLE

using namespace std;

long long time_now()
{
#ifdef _WIN32
	return clock();
#else
	timeval t;
	gettimeofday(&t,NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
#endif
}

int DedicatedServer::nextPlayerID()
{
	return playerIDGenerator.nextID();
}

DedicatedServer::DedicatedServer():
	fps_world(0)
{
	visualworld.disable(); // server doesnt need visual information
	playerIDGenerator.setNextID(0);
	
	pause_state = WAITING_PLAYERS;
	serverAllow = 0;
	init();
	
	load("server.conf");
	
	areas.insert(make_pair("default_area", World(&visualworld)));
	areas.find("default_area")->second.buildTerrain(1);
	
	areas.insert(make_pair("other_area", World(&visualworld)));
	areas.find("other_area")->second.buildTerrain(5);
}

void DedicatedServer::init()
{
	long long milliseconds = time_now();
	fps_world.reset(milliseconds);
	
	srand(time(0));
	
	// TODO: This should definetly not be necessary to do in the server :D
	ModelFactory::load(World::BULLET_MODEL, "", "");
	ModelFactory::load(World::PLAYER_MODEL, "", ""); // TODO: (pre)loading any skeletalmodel in server fails because OpenGL is not initialized.
	ModelFactory::load(World::ZOMBIE_MODEL, "", "");
	ModelFactory::load(World::INVISIBLE_MODEL, "", "");
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
	discCommand << -1 << " " << (serverAllow + 1) << " 100 " << leaver << "#";
	serverMsgs.push_back( discCommand.str() );
	
	cerr << "Saving disconnecting character with key: " << Players[leaver].key << endl;
	
	CharacterInfo& c = dormantPlayers[Players[leaver].key];
	c.playerInfo = Players[leaver];
	
	cerr << "DISCONNECT: " << Players[leaver].name << " " << Players[leaver].kills << " " << Players[leaver].deaths << endl;
	
	for(auto iter = areas.begin(); iter != areas.end(); iter++)
	{
		auto unit = iter->second.units.find(leaver);
		if(unit != iter->second.units.end())
			c.unit = unit->second;
	}
	
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
	
	// prepare to update game world.
	processClientMsgs();
	
	
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
			#ifdef SLEEP_IF_POSSIBLE
			usleep(1000);
			#endif
			
			return;
		}
	}
	
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
		if(intVals["AUTO_SPAWN"])
		{
			if((simulRules.currentFrame % intVals["AUTO_SPAWN_RATE"]) == 0)
			{
				serverSendMonsterSpawn(); // spawn a monster every now and then.
			}
		}
		
		simulateWorldFrame();
	}
	else
	{
		// this might not be a good idea in the end, but should show us how much it really uses processing power.
		#ifdef SLEEP_IF_POSSIBLE
		usleep(1000);
		#endif
	}
}

void DedicatedServer::simulateWorldFrame()
{
	if( (UnitInput.back().plr_id == SERVER_ID) && (UnitInput.back().frameID != simulRules.currentFrame) )
		cerr << "ERROR: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;
	
	fps_world.insert();
	
	/*
	static World::CheckSumType checksums[10] = { 0 };
	int current = simulRules.currentFrame % 10;
	checksums[current] = world.checksum();
	*/
	
	while(!UnitInput.empty() && UnitInput.back().frameID == simulRules.currentFrame)
	{
		Order tmp = UnitInput.back();
		UnitInput.pop_back();
		
		if(tmp.plr_id == SERVER_ID)
		{
			cerr << "MOTHERFUCKER FUCKING FUCK YOU MAN?= JUST FUCK YOU!!" << endl;
			break;
		}

		/*
		World::CheckSumType cs_tmp = checksums[(tmp.frameID + 10 - 5) % 10];
		if((simulRules.currentFrame > 10) && (tmp.checksum != cs_tmp) && (tmp.checksum != 0))
		{
			std::cerr << "OUT OF SYNC: player " << tmp.plr_id << " frame: " << tmp.frameID << std::endl;
			std::cerr << "client checksum: " << tmp.checksum << std::endl;
			std::cerr << "server checksum: " << checksums[(tmp.frameID + 10 - 5) % 10] << std::endl;
			cerr << "server unit locations:" << std::endl;
			for (auto it = world.units.begin(); it != world.units.end(); ++it)
			{
				int id = it->first;
				const Location& pos = it->second.getPosition();
				cerr << id << ": " << pos << std::endl;
			}
			
			pause_state = PAUSED;
		}
		*/
		
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			auto it = world.units.find(tmp.plr_id);
			
			if(it != world.units.end())
			{
				it->second.updateInput(tmp.keyState, tmp.mousex, tmp.mousey, tmp.mouseButtons);
			}
		}
	}
	
	if(!UnitInput.empty() && UnitInput.back().frameID < simulRules.currentFrame)
	{
		cerr << "Server has somehow skipped an order :(" << endl;
		UnitInput.pop_back();
	}
	
	for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
	{
		World& world = area_it->second;
		world.worldTick(simulRules.currentFrame);
	}
	
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
			
			if(player.name == "" || player.name == "nameless")
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
		else if(cmd == "SWARM")
		{
			int n = 70;
			int team = -1;
			if(ss >> n)
				ss >> team;
			
			n = min(100, n);
			
			serverSendMonsterSpawn(n, team);
			
			stringstream chatmsg;
			chatmsg << "3 -1 ^r" << player.name << "^w has performed a dark ritual.. the ^rswarm ^wis anigh..#";
			serverMsgs.push_back(chatmsg.str());
			
			return;
		}
		else if(cmd == "TEAM")
		{
			int new_team = 0;
			ss >> new_team;
			
			stringstream team_change;
			team_change << "-1 " << (serverAllow + 10) << " 11 " << id << " " << new_team << "#";
			serverMsgs.push_back(team_change.str());
			return;
		}
		else if(cmd == "AUTO_SPAWN")
		{
			intVals["AUTO_SPAWN"] ^= 1;
			
			if(intVals["AUTO_SPAWN"])
			{
				stringstream autospawn;
				autospawn << "3 -1 ^YAutospawn toggled ^GON#";
				serverMsgs.push_back(autospawn.str());
			}
			else
			{
				stringstream autospawn;
				autospawn << "3 -1 ^YAutospawn toggled ^ROFF#";
				serverMsgs.push_back(autospawn.str());
			}
			
			return;
		}
	}
	else if(orderWord == "-1")
	{
		// WTF?? Someone is trying to impersonate GOD (that's me). Maybe I should lay some thunder on his ass?
		cerr << "SOMEONE IS IMPERSONATING GOD!! :" << msg << endl;
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
	static int id = nextPlayerID();
	if(sockets.accept(id))
	{
		cerr << "looks like someone is connecting :O" << endl;
		Players[id].connectionState = 0;
		id = nextPlayerID();
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
	
	else if(server_msg.serverCommand == 10) // spawn monster to team -1
	{
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			world.addUnit(world.nextUnitID(), false);
		}
	}
	else if(server_msg.serverCommand == 11) // change team message
	{
		int unitID     = server_msg.keyState;
		int new_teamID = server_msg.mousex;
		
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			
			auto unit_it = world.units.find(unitID);
			if(unit_it != world.units.end())
			{
				Unit& u = unit_it->second;
				u["TEAM"] = new_teamID;
			}
		}
	}
	else if(server_msg.serverCommand == 15) // spawn monster to a specific team
	{
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			world.addUnit(world.nextUnitID(), false, server_msg.keyState);
		}
	}
	else if(server_msg.serverCommand == 100) // SOME PLAYER HAS DISCONNECTED
	{
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			world.removeUnit(server_msg.keyState);
		}
		
		simulRules.numPlayers--;
	}
	
	else if(server_msg.serverCommand == 1) // ADDHERO message
	{
		World& world = areas.find("default_area")->second;
		
		world.addUnit(server_msg.keyState);
		
		// the new way
		if(SpawningHeroes[server_msg.keyState].unit.name == "nameless")
		{
			SpawningHeroes[server_msg.keyState].unit.name = Players[server_msg.keyState].name;
			SpawningHeroes[server_msg.keyState].playerInfo.name = Players[server_msg.keyState].name;
		}
		
		SpawningHeroes[server_msg.keyState].unit.birthTime = world.units[server_msg.keyState].birthTime;
		SpawningHeroes[server_msg.keyState].unit.id = server_msg.keyState;
		
		world.units.find(server_msg.keyState)->second = SpawningHeroes[server_msg.keyState].unit;
		Players[server_msg.keyState] = SpawningHeroes[server_msg.keyState].playerInfo;
		
		SpawningHeroes.erase(server_msg.keyState);
		
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
		
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
		
			// set unit's name to match the players
			if(world.units.find(plrID) != world.units.end())
			{
				world.units[plrID].name = Players[plrID].name;
			}
		}
	}
	
	else if(order_type == -1) // A COMMAND message from GOD (server)
	{
		
		Order tmp_order;
		tmp_order.plr_id = -1;
		ss >> tmp_order.frameID;
		ss >> tmp_order.serverCommand;
		ss >> tmp_order.keyState;
		ss >> tmp_order.mousex >> tmp_order.mousey;
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
			cerr << "server received a unit copy message. makes no sense?" << endl;
			
			/*
			cerr << "Creating a new unit as per instructions" << endl;
			int unitID;
			ss >> unitID;
			world.addUnit(unitID);
			world.units[unitID].handleCopyOrder(ss);
			*/
		}
		else if(cmd == "PROJECTILE")
		{
			cerr << "server received a projectile copy message. makes no sense?" << endl;
			
			/*
			int id;
			size_t prototype_model;
			ss >> id >> prototype_model;
			Location dummy;
			world.addProjectile(dummy, id, prototype_model);
			world.projectiles[id].handleCopyOrder(ss);
			*/
		}
		else if(cmd == "NEXT_UNIT_ID")
		{
			cerr << "server received next unit id message. makes no sense?" << endl;
			
			/*
			int id = -1;
			ss >> id;
			world.setNextUnitID(id);
			*/
		}
		else if(cmd == "SIMUL")
		{
			cerr << "server received an order to set simulation rules according to client wishes. ignoring order." << endl;
			
			/*
			cerr << "Set simulRules to instructed state" << endl;
			ss >> simulRules.currentFrame >> simulRules.windowSize >> simulRules.frameSkip >> simulRules.numPlayers >> simulRules.allowedFrame;
			cerr << simulRules.currentFrame << " " << simulRules.windowSize << " " << simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << endl;
			*/
		}
		else if(cmd == "CLIENT_STATE")
		{
			int nopause = 0;
			ss >> nopause;
			pause_state = (nopause ? RUNNING : PAUSED);
			
			long long milliseconds = time_now();
			fps_world.reset(milliseconds);
		}
		else if(cmd == "CHAR_INFO")
		{
			int future_player_id;
			ss >> future_player_id;
			
			string line;
			getline(ss, line);
			
			CharacterInfo ci;
			ci.readDescription(line);
			
			SpawningHeroes[future_player_id] = ci;
		}
		else
		{
			cerr << "INSTANT ORDER FUCK!" << endl;
		}
		
	}
	else if(order_type == -4) // copy of an existing order
	{
		cerr << "Server got a copy of an old message?? makes no sense." << endl;
		
		//Order tmp_order;
		//ss >> tmp_order.frameID >> tmp_order.plr_id >> tmp_order.keyState >> tmp_order.mousex >> tmp_order.mousey >> tmp_order.serverCommand >> tmp_order.mouseButtons;
		//UnitInput.push_back(tmp_order);
	}
	else
	{
		cerr << "WARNING: Server was unable to handle a received order of type " << order_type << endl;
	}
}


void DedicatedServer::handleWorldEvents()
{
	// output events to show the server is still in sync.
	for(size_t i = 0; i < visualworld.events.size(); ++i)
	{
		WorldEvent& event = visualworld.events[i];
		
		if(event.type == WorldEvent::DEATH_ENEMY)
		{
			if( (Players.find(event.actor_id) != Players.end()) )
			{
				Players[event.actor_id].kills++;
				cerr << Players[event.actor_id].name << " has killed" << endl;
			}
		}
		
		if(event.type == WorldEvent::DEATH_PLAYER)
		{
			if( (Players.find(event.actor_id) != Players.end()) )
			{
				Players[event.actor_id].kills++;
				cerr << Players[event.actor_id].name << " has killed" << endl;
			}
			
			if( (Players.find(event.target_id) != Players.end()) )
			{
				Players[event.target_id].deaths++;
				cerr << Players[event.target_id].name << " has died" << endl;
			}
		}
	}
	
	visualworld.events.clear();
}

