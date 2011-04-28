#include "dedicated.h"

#include "timer.h"
#include "vec3.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <omp.h>

#define SLEEP_IF_POSSIBLE
#define MULTI_THREADED_WORLD_TICKS 0

using namespace std;
using namespace Network;

void DedicatedServer::possible_sleep(int milliseconds) const
{
#ifdef SLEEP_IF_POSSIBLE
	Timer::sleep(milliseconds);
#endif
}

int DedicatedServer::nextPlayerID()
{
	return playerIDGenerator.nextID();
}

DedicatedServer::DedicatedServer():
	checkSumVectorSize(150),
	fps_world(0)
{
	playerIDGenerator.setNextID(0);
	
	pause_state = WAITING_PLAYERS;
	serverAllow = 0;
	
	visualworld.disable();
	
	load("configs/server.conf");
	
	// TODO: Make a separate function for these things.
	string first_area = "default_area";
	string second_area = "other_area";
	
	if(intVals.find("SERVER_NO_WAIT") != intVals.end())
	{
		server_no_wait = intVals["SERVER_NO_WAIT"];
	}
	else
	{
		server_no_wait = false;
	}
	
	float world_gen_percentage = 0.0f;
	
	areas.insert(make_pair(first_area, World(&visualworld)));
	areas.find(first_area)->second.buildTerrain(15, world_gen_percentage);
	visualworld.disable(); // server doesnt need visual information
	
	areas.find(first_area)->second.load("serverdata/worlds/default_area.dat");
	area_settings[first_area].load("serverdata/worlds/default_area.area");
	areas.find(first_area)->second.strVals["AREA_NAME"] = first_area;
	
	int portal1_id = areas.find(first_area)->second.nextUnitID();
	areas.find(first_area)->second.addItem(Location(200, 50, 200), Location(0, 0, 0), portal1_id);
	WorldItem& portal1 = areas.find(first_area)->second.items[portal1_id];
	portal1.intVals["PERSISTS"] = 1;
	portal1.intVals["AREA_CHANGE"] = 1;
	portal1.strVals["AREA_CHANGE_TARGET"] = second_area;
	
	
	areas.insert(make_pair(second_area, World(&visualworld)));
	areas.find(second_area)->second.buildTerrain(5, world_gen_percentage);
	visualworld.disable(); // server doesnt need visual information
	
	areas.find(second_area)->second.load("serverdata/worlds/other_area.dat");
	area_settings[second_area].load("serverdata/worlds/other_area.area");
	areas.find(second_area)->second.strVals["AREA_NAME"] = second_area;
	
	int portal2_id = areas.find(second_area)->second.nextUnitID();
	areas.find(second_area)->second.addItem(Location(700, 50, 700), Location(0, 0, 0), portal2_id);
	WorldItem& portal2 = areas.find(second_area)->second.items[portal2_id];
	portal2.intVals["PERSISTS"] = 1;
	portal2.intVals["AREA_CHANGE"] = 1;
	portal2.strVals["AREA_CHANGE_TARGET"] = first_area;
	
	init();
}

void DedicatedServer::init()
{
	visualworld.disable();
	
	long long milliseconds = Timer::time_now();
	fps_world.reset(milliseconds);
	
	// srand(time(0));
	
	// init checksums
	for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
	{
		checkSums[area_it->first].resize(checkSumVectorSize); // gives about three seconds to clients.
	}
	
	
	stringstream createBaseBuildings;
	createBaseBuildings << MessageType::SERVER_ORDER << " " << (serverAllow + 10) << " 18#";
	serverMsgs.push_back(createBaseBuildings.str());
}

void DedicatedServer::send_to_all(const std::string& msg)
{
	for(auto it = Players.begin(); it != Players.end(); ++it)
	{
		// dont write to disconnected players
		if(sockets.alive(it->first) && it->second.connectionState == ConnectionState::GAMEPLAY)
		{
			sockets.getConnection(it->first) << msg;
		}
	}
}

bool DedicatedServer::start(int port)
{
	return sockets.listen(port);
}

void DedicatedServer::disconnect(int leaver)
{
	if(Players.find(leaver) == Players.end())
	{
		cerr << "Trying to disconnect a player that doesn't exist! Breaking attempt." << endl;
		return;
	}
	
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
			leaver = i->first;
			continue;
		}
		
		int conn_state = Players[i->first].connectionState;
		SocketHandler::Connection& conn = sockets.getConnection(i->first);
		string msg;
		
		if(!conn.empty())
		{
			switch(conn_state)
			{
				case ConnectionState::SIGN_IN:
				{
					while(conn >> msg)
						handleSignInMessage(i->first, msg);
					break;
				}
				
				case ConnectionState::GAMEPLAY:
				{
					while(conn >> msg)
						parseClientMsg(msg, i->first, i->second);
					break;
				}
				
				case ConnectionState::WAIT_WORLD_GEN:
				{
					// NOTE: duplicate for now, should not be.
					while(conn >> msg)
						parseClientMsg(msg, i->first, i->second);
					break;
				}
				
				case ConnectionState::OBSERVER:
				{
					// ..
					
					break;
				}
				
				case ConnectionState::ADMIN:
				{
					while(conn >> msg)
						parseAdminMsg(msg, i->first, i->second);
					break;
				}
			}
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
	if(server_no_wait)
		--minAllowed;
	
	for(auto it = Players.begin(); it != Players.end(); ++it)
	{
		if(it->second.connectionState == ConnectionState::GAMEPLAY && (it->second.last_order < minAllowed))
			minAllowed = it->second.last_order;
	}
	
	if(minAllowed < simulRules.windowSize)
		minAllowed = simulRules.windowSize;
	
	if(minAllowed != UINT_MAX)
	{
		simulRules.allowedFrame = minAllowed;
		
		if(minAllowed > simulRules.currentFrame)
			minAllowed = simulRules.currentFrame;
		
		if( pause_state == RUNNING && (minAllowed > serverAllow) )
		{
			stringstream allowSimulation_msg;
			allowSimulation_msg << "-2 ALLOW " << minAllowed << "#";
			
			serverAllow = minAllowed;
			serverMsgs.push_back(allowSimulation_msg.str());
		}
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
			possible_sleep(10);
			
			return;
		}
	}

	// TODO: THIS DOESNT ACTUALLY WORK (I THINK). THE ONLY REASON THE SERVER WORKS, IS THAT THIS CODE IS _NEVER_ EXECUTED
	// the level can kind of shut down when there's no one there.
	if( Players.empty() && UnitInput.empty() )
	{
		// cerr << "waiting ..." << endl;
		
		possible_sleep(1);
		
		return;
		
		simulRules.reset();
		
		long long milliseconds = Timer::time_now();
		fps_world.reset(milliseconds);
		
		cerr << "World shutting down." << endl;
		pause_state = WAITING_PLAYERS;
		return;
	}
	
	processServerMsgs();

	long long milliseconds = Timer::time_now();
	
	if( (simulRules.currentFrame < simulRules.allowedFrame) && fps_world.need_to_draw(milliseconds) )
	{
		simulateWorldFrame();
		
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			map<int, Unit>& units = area_it->second.units;
			
			for(auto unit_it = units.begin(); unit_it != units.end(); unit_it++)
			{
				if(unit_it->first >= 10000)
					break;
				
				if(unit_it->second.intVals["NEEDS_AREA_TRANSFER"] == 1)
				{
					unit_it->second.intVals["NEEDS_AREA_TRANSFER"] = 0;
					startPlayerAreaChange(unit_it->second.strVals["NEXT_AREA"], unit_it->first);
					break;
				}
			}
		}
	}
	else
	{
		// this might not be a good idea in the end, but should show us how much it really uses processing power.
		possible_sleep(1);
	}

}


bool DedicatedServer::checkSumOK(int plrID, int frameID, vector<World::CheckSumType>& plr_cs)
{
	
	for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
	{
		assert(checkSumVectorSize == 150);
		assert(checkSums[area_it->first].size() == unsigned(checkSumVectorSize));
		
		if(area_it->second.units.find(plrID) != area_it->second.units.end())
		{
			assert(checkSums.find(area_it->first) != checkSums.end());
			
			bool all_is_fine = true;
			vector<World::CheckSumType>& checksum_vector = checkSums[area_it->first][frameID % checkSumVectorSize];
			
			
			// cerr << "checksum size: " << plr_cs.size() << " " << checksum_vector.size() << endl;
			assert(plr_cs.size() == checksum_vector.size());
			
			assert(Players.find(plrID) != Players.end());
			
			for(size_t i = 0; i < checksum_vector.size(); ++i)
			{
				if(checksum_vector[i] != plr_cs[i])
				{
					vector<string> reasons;
					reasons.push_back("unit positions and ids");
					reasons.push_back("unit hitpoints");
					reasons.push_back("item positions");
					reasons.push_back("projectile positions");
					
					cerr << "out of sync! reason: \"" << reasons[i] << "\", frame: " << frameID << ", server cs: " << checksum_vector[i] << ", client cs: " << plr_cs[i] << endl;
					
					stringstream chat_msg;
					chat_msg << "3 -1 ^G" << Players[plrID].name << " ^Rout of sync: " << reasons[i] << "#";
					serverMsgs.push_back(chat_msg.str());
					
					all_is_fine = false;
				}
			}
			
			/*
			if( (simulRules.currentFrame > checkSumVectorSize) && (!all_is_fine))
			{
				stringstream chat_msg;
				chat_msg << "3 -1 ^G" << Players[plrID].name << " ^Rout of sync: DISCONNECTED#";
				serverMsgs.push_back(chat_msg.str());
				cerr << chat_msg.str() << endl;
				disconnect(plrID);
			}
			*/
			
			// no worries! all is fine!
			return all_is_fine;
		}
	}
	
	// if player character is nowhere, how can?
	return true;
}



void DedicatedServer::simulateWorldFrame()
{
	
	
	if(!UnitInput.empty())
		if( (UnitInput.back().plr_id == SERVER_ID) && (UnitInput.back().frameID != simulRules.currentFrame) )
			cerr << "WARNING: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;
	
	sort(UnitInput.begin(), UnitInput.end());
	
	while(!UnitInput.empty() && UnitInput.back().frameID < simulRules.currentFrame)
	{
		cerr << "Server has somehow skipped an order for frame: " << UnitInput.back().frameID << ", current frame = " << simulRules.currentFrame << ", source = " << Players[UnitInput.back().plr_id].name << endl;
		UnitInput.pop_back();
	}
	
	while(!UnitInput.empty() && UnitInput.back().frameID == simulRules.currentFrame)
	{
		Order tmp = UnitInput.back();
		UnitInput.pop_back();
		
		if(tmp.plr_id == SERVER_ID)
		{
			cerr << "ERROR: Server processing order messages and encountered a server message" << endl;
			continue;
		}
		
		// the actual update code
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
	
	
	fps_world.insert();
	
#if MULTI_THREADED_WORLD_TICKS > 0
	// build something that is easier to iterate over.
	vector<World*> worlds;
	for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
	{
		worlds.push_back(&(area_it->second));
	}
	
	int num_worlds = worlds.size();
	int current_world_num = 0;
	
	// TODO ALERT: Move event container from visual world to world.
	//             Because server has a shared visual world,
	//             things will be fucked up when 8 threads are writing there
	//             concurrently.
	#pragma omp parallel for shared(num_worlds) schedule(dynamic)
	for(current_world_num = 0; current_world_num < num_worlds; current_world_num++)
	{
		worlds[current_world_num]->worldTick(simulRules.currentFrame);
	}
#else
	for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
	{
		World& world = area_it->second;
		world.worldTick(simulRules.currentFrame);
	}
#endif
	
	handleWorldEvents();
	
	int current = simulRules.currentFrame % checkSumVectorSize;
	for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
	{
		checkSums[area_it->first][current].clear();
		area_it->second.checksum(checkSums[area_it->first][current]);
	}
	
	simulRules.currentFrame++;
}





bool DedicatedServer::startPlayerAreaChange(const string& next_area, int player_id)
{
	if(areas.find(next_area) == areas.end())
	{
		stringstream chat_msg;
		chat_msg << "3 -1 ^RUnable to apply area change, area \"^G" << next_area << "^R\" does not exist.#";
		serverMsgs.push_back(chat_msg.str());
		return false;
	}
	
	for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
	{
		auto unit_it = area_it->second.units.find(player_id);
		if(unit_it != area_it->second.units.end())
		{
			
			stringstream chat_msg;
			chat_msg << "3 -1 ^GFound unit and target area! Sending area change kill message.#";
			serverMsgs.push_back(chat_msg.str());
			
			unit_it->second.strVals["NEXT_AREA"] = next_area;
			unit_it->second.updateInput(0, 0, 0, 0);
			
			// send area change kill message
			stringstream area_change_msg;
			area_change_msg << "-1 " << (serverAllow + 1) << " 7 " << player_id << "#";
			serverMsgs.push_back(area_change_msg.str());
			return true;
		}
	}
	
	// send an error message to players.
	stringstream chat_msg;
	chat_msg << "3 -1 ^RDid not find unit for area change! This should never happen!#";
	serverMsgs.push_back(chat_msg.str());
	return false;
}


void DedicatedServer::parseAdminMsg(const std::string& msg, int admin_id, PlayerInfo& admin)
{
	string new_message = msg + "#";
	stringstream ss(new_message);
	
	int orderType;
	ss >> orderType;
	
	switch(orderType)
	{
		case MessageType::ADMIN_ORDER_MESSAGE:
		{
			// ...
			
			// to make it compile..
			admin_id++;
			admin.last_order++;
			
			break;
		}
		default:
		{
			cerr << "WARNING: Admin connection sent an unrecognized command: " << msg << endl;
		}
	}
}

void DedicatedServer::parseClientMsg(const std::string& msg, int player_id, PlayerInfo& player)
{
	string new_message = msg + "#";
	// keep track of last frames for which orders have been received.
	stringstream ss(new_message);
	
	
	int orderType;
	ss >> orderType;
	
	if(orderType == MessageType::PLAYER_INPUT)
	{
		if(player.connectionState != ConnectionState::GAMEPLAY)
		{
			cerr << "WARNING: Ignoring a player input command. Arrived from a source with a wrong connectionstate." << endl;
			return;
		}
		
		if(server_no_wait)
		{
			// recreate the message
			Order tmp_order;
			ss >> tmp_order.plr_id;
			ss >> tmp_order.frameID;
			ss >> tmp_order.keyState;
			ss >> tmp_order.mousex >> tmp_order.mousey;
			ss >> tmp_order.mouseButtons;
			
			if(player.last_order < simulRules.currentFrame + 5)
			{
				// increasing player's order frameskip to compensate for lag.
				player.last_order = simulRules.currentFrame + 5;
			}
			
			tmp_order.frameID = ++player.last_order;
			
			stringstream order_msg;
			order_msg << "1 " << tmp_order.plr_id << " " << tmp_order.frameID << " " << 
			tmp_order.keyState << " " << tmp_order.mousex << " " << tmp_order.mousey << " " << tmp_order.mouseButtons << "#";
			
			new_message = order_msg.str();
		}
		else
		{
			int plr_id;
			unsigned frameID;
			
			ss >> plr_id >> frameID;
			
			// ignore order if it's in the past.
			if(frameID <= simulRules.currentFrame)
			{
				cerr << "WARNING: Server intentionally skipping an order, it arrived too late!" << endl;
				return;
			}
			
			player.last_order = frameID;
		}
	}
	else if(orderType == MessageType::PLAYERINFO_MESSAGE)
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
			new_message_ss << MessageType::PLAYERINFO_MESSAGE << " " << id << " " << Players[id].kills << " " << Players[id].deaths << " " << Players[id].name << "#";

			new_message = new_message_ss.str();
		}
	}
	else if(orderType == MessageType::CHAT_MESSAGE) // chat message!
	{
		string id;
		string cmd;
		
		ss >> id >> cmd;
		
		cerr << "Chat <" << id << "> " << cmd << endl;
		
		if(cmd == "SWARM")
		{
			int n = 70;
			int team = -1;
			if(ss >> n)
				ss >> team;
			
			n = min(100, n);
			
			serverSendMonsterSpawn(n, team);
			
			stringstream chatmsg;
			chatmsg << MessageType::CHAT_MESSAGE << " " << SERVER_ID << " ^r" << player.name << "^w has performed a dark ritual.. the ^rswarm ^wis anigh..#";
			serverMsgs.push_back(chatmsg.str());
			
			return;
		}
		else if(cmd == "GODMODE")
		{
			string property;
			int value;
			
			ss >> property >> value;
			
			stringstream property_msg;
			property_msg << MessageType::SERVER_ORDER << " " << (serverAllow + 10) << " 24 " << id << "#";
			serverMsgs.push_back(property_msg.str());
			
			return;
		}
		else if(cmd == "TEAM")
		{
			int new_team = 0;
			ss >> new_team;
			
			stringstream team_change;
			team_change << MessageType::SERVER_ORDER << " " << (serverAllow + 10) << " 11 " << id << " " << new_team << "#";
			serverMsgs.push_back(team_change.str());
			return;
		}
		else if(cmd == "AUTO_SPAWN")
		{
			stringstream autospawn;
			autospawn << MessageType::CHAT_MESSAGE << " " << SERVER_ID << " ^YAutospawn functionality is ^RDEPRECATED#";
			serverMsgs.push_back(autospawn.str());
			return;
		}
		else if(cmd == "AREA")
		{
			string next_area;
			ss >> next_area;
			
			if(next_area[next_area.size() - 1] == '#')
				next_area.resize(next_area.size() - 1);
			
			startPlayerAreaChange(next_area, player_id);
			return;
		}
		
	}
	else if(orderType == MessageType::SERVER_ORDER)
	{
		// WTF?? Someone is trying to impersonate GOD (that's me). Maybe I should lay some thunder on his ass?
		cerr << "SOMEONE IS IMPERSONATING GOD!! :" << msg << endl;
		new_message = "";
		
		// disconnect the fucker
		disconnect(player_id);
		
		stringstream chatmsg;
		chatmsg << MessageType::CHAT_MESSAGE << " " << SERVER_ID << " ^r" << player.name << " ^w has attempted to impersonate ^GME ^w .. --> ^Rdisconnected#";
		serverMsgs.push_back(chatmsg.str());
	}
	else if(orderType == MessageType::INSTANT_REACTION) // instant reaction message from client
	{
		string cmd;
		ss >> cmd;
		
		if(cmd == "WORLD_GEN_READY")
		{
			cerr << "Client claims to be finished with world gen! Sending world content.." << endl;
			sendWorldContent(player_id);
			cerr << "Done" << endl;
			return;
		}
		if(cmd == "CSMSG")
		{
			int frameID;
			ss >> frameID;
			
			vector<World::CheckSumType> plr_cs;
			World::CheckSumType cs_tmp;
			while(ss >> cs_tmp)
				plr_cs.push_back(cs_tmp);
			checkSumOK(player_id, frameID, plr_cs);
			return;
		}
		else
		{
			cerr << "Unknown instant order from client: " << ss.str() << endl;
			return;
		}
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
		Players[id].connectionState = ConnectionState::SIGN_IN;
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
		
		// TODO: IF THE PLAYER DID NOT START ACTUALLY PLAYING (DISCONNECT AT HERO SELECTION)
		// THIS WILL FUCK THINGS UP
		// NUM PLAYERS SHOULD BE DETERMINABLE FROM Players
		// (loop through and see how many are at connectionState GAMEPLAY)
		
		simulRules.numPlayers--;
	}
	else if(server_msg.serverCommand == 18) // create base buildings
	{
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			world.resetGame();
		}
	}
	else if(server_msg.serverCommand == 1) // ADDHERO message
	{
		string areaName = SpawningHeroes[server_msg.keyState].unit.strVals["AREA"];
		World& world = areas.find(areaName)->second;
		
		world.addUnit(server_msg.keyState);
		
		// the new way
		if(SpawningHeroes[server_msg.keyState].unit.name == "nameless")
		{
			SpawningHeroes[server_msg.keyState].unit.name = Players[server_msg.keyState].name;
			SpawningHeroes[server_msg.keyState].playerInfo.name = Players[server_msg.keyState].name;
		}
		
		SpawningHeroes[server_msg.keyState].unit.birthTime = world.units[server_msg.keyState].birthTime;
		SpawningHeroes[server_msg.keyState].unit.id = server_msg.keyState;
		
		Unit& unit = world.units.find(server_msg.keyState)->second;
		unit = SpawningHeroes[server_msg.keyState].unit;
		// unit.intVals["NO_ORDERS"] = 80; // NOTE: to prevent de-sync!
		
		
		Players[server_msg.keyState] = SpawningHeroes[server_msg.keyState].playerInfo;
		Players[server_msg.keyState].last_order = simulRules.currentFrame + simulRules.windowSize;
		
		SpawningHeroes.erase(server_msg.keyState);
		
		cerr << "Adding a new hero at frame " << simulRules.currentFrame << ", units.size() = " << world.units.size() << endl;
		
		sort(UnitInput.begin(), UnitInput.end());
		serverSendRequestPlayerNameMessage(server_msg.keyState);
	}
	else if(server_msg.serverCommand == 2) // "set playerID" message
	{
		cerr << "SERVER HAS NO ID, CAN NOT CHANGE IT" << endl;
	}
	else if(server_msg.serverCommand == 24) // toggle god-mode
	{
		int id = server_msg.keyState;
		
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			auto unit_it = world.units.find(id);
			if(unit_it != world.units.end())
			{
				unit_it->second.intVals["GOD_MODE"] ^= 1;
				break;
			}
		}
		
	}
	else if(server_msg.serverCommand == 7) // destroy hero, for area change -message
	{
		// so what we actually want to do, is save the hero into dormantPlayers, then removeUnit,
		// (by now the unit's NEXT_AREA strVal should have been set to it's new value, move this to AREA)
		// then send area change message to the target.
		
		int destroy_ID = server_msg.keyState;
		
		for(auto area_it = areas.begin(); area_it != areas.end(); area_it++)
		{
			World& world = area_it->second;
			auto unit_it = world.units.find(destroy_ID);
			if(unit_it != world.units.end())
			{
				// found it!
				unit_it->second.strVals["AREA"] = unit_it->second.strVals["NEXT_AREA"];
				
				dormantPlayers[Players[destroy_ID].key].unit = unit_it->second;
				dormantPlayers[Players[destroy_ID].key].playerInfo = Players[destroy_ID];
				
				world.removeUnit(destroy_ID);
				break;
			}
		}
		
		int orders_destroyed = 0;
		for(size_t i = 0; i < UnitInput.size(); i++)
		{
			if(UnitInput[i].plr_id == destroy_ID)
			{
				orders_destroyed++;
				
				UnitInput[i] = UnitInput.back();
				UnitInput.pop_back();
				--i;
			}
		}
		
		cerr << "Destroyed " << orders_destroyed << " orders from area change fellow." << endl;
		sort(UnitInput.begin(), UnitInput.end());
		
		// this should be enough
		changeArea(destroy_ID);
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

		processClientMsg(clientOrders.orders[i]); // TODO: the ID is implicit! should give it as parameter here!
		
	}
	
	clientOrders.orders.clear();
}

void DedicatedServer::processClientMsg(const std::string& msg)
{
	stringstream ss(msg);
	
	int order_type;
	ss >> order_type;
	
	if(order_type == MessageType::PLAYER_INPUT) // ordinary input order
	{
		Order tmp_order;
		ss >> tmp_order.plr_id;
		ss >> tmp_order.frameID;
		ss >> tmp_order.keyState;
		ss >> tmp_order.mousex >> tmp_order.mousey;
		ss >> tmp_order.mouseButtons;
		
		UnitInput.push_back(tmp_order);
	}
	else if(order_type == MessageType::PLAYERINFO_MESSAGE) // playerInfo message
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
	
	else if(order_type == MessageType::SERVER_ORDER) // A COMMAND message from GOD (server)
	{
		
		Order tmp_order;
		tmp_order.plr_id = SERVER_ID;
		ss >> tmp_order.frameID;
		ss >> tmp_order.serverCommand;
		ss >> tmp_order.keyState;
		ss >> tmp_order.mousex >> tmp_order.mousey;
		UnitInput.push_back(tmp_order);
	}
	
	
	else if(order_type == MessageType::INSTANT_REACTION) // instant reaction message from GOD
	{
		string cmd;
		ss >> cmd;
		if(cmd == "GO")
		{
			cerr << "Setting pause state to RUNNING" << endl;
			pause_state = RUNNING;
			
			long long milliseconds = Timer::time_now();
			fps_world.reset(milliseconds);
		}
		else if(cmd == "ALLOW")
		{
			int frame;
			ss >> frame;
			simulRules.allowedFrame = frame;
		}
		else if(cmd == "GIVE_NAME") // request player name message
		{
			cerr << "I AM THOR!" << endl;
		}
		else if(cmd == "UNIT") // unit copy message
		{
			cerr << "server received a unit copy message. makes no sense?" << endl;
		}
		else if(cmd == "PROJECTILE")
		{
			cerr << "server received a projectile copy message. makes no sense?" << endl;
		}
		else if(cmd == "NEXT_UNIT_ID")
		{
			cerr << "server received next unit id message. makes no sense?" << endl;
		}
		else if(cmd == "SIMUL")
		{
			cerr << "server received an order to set simulation rules according to client wishes. ignoring order." << endl;
		}
		else if(cmd == "CLIENT_STATE")
		{
			int nopause = 0;
			ss >> nopause;
			pause_state = (nopause ? RUNNING : PAUSED);
			
			long long milliseconds = Timer::time_now();
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
			cerr << "Unrecognized instant order: \"" << msg << "\"" << endl;
		}
		
	}
	else if(order_type == MessageType::COPY_ORDER_MESSAGE) // copy of an existing order
	{
		cerr << "Server got a copy of an old message?? makes no sense." << endl;
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

