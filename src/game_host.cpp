
#include "game.h"

using namespace std;


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
			for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
				target->second.write(i->second.msgs[k]);
			
			// keep track of last frames for which orders have been received.
			stringstream ss(i->second.msgs[k]);
			string orderWord;
			ss >> orderWord;
			
			if(orderWord == "1")
			{
				int order_player_id, frame;
				ss >> order_player_id >> frame;
				i->second.last_order = frame;
				
				// cerr << "FINISHED MESSAGE (" << frame << "): " << i->second.msgs[k] << endl;
			}
		}
		i->second.msgs.clear();
	}
	
	if(leaver != -1) // there is a leaver!!
	{
		// TODO
		stringstream discCommand;
		discCommand << -1 << " " << (sockets.sockets[leaver].last_order + 1) << " 100 " << leaver << "#";
		serverMsgs.push_back( discCommand.str() );
		
		sockets.erase_id(leaver);
	}
	
	unsigned minAllowed = ~0;
	for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
	{
		if(target->second.last_order < minAllowed)
			minAllowed = target->second.last_order;
	}
	
	static int allow = 1;
	if(userio.getSingleKey() == "return")
	{
		allow ^= 1;
	}
	
	if(minAllowed < 5)
		minAllowed = 5;
	
	if( (minAllowed != ~0) && (minAllowed > serverAllow) )
	{
		if(allow == 1)
		{
			stringstream allowSimulation_msg;
			allowSimulation_msg << "-2 ALLOW " << minAllowed << "#";
			
			// cerr << "minAllowed: " << minAllowed << ", serverAllow: " << serverAllow << endl;
			
			serverAllow = minAllowed;
			serverMsgs.push_back(allowSimulation_msg.str());
		}
	}
	
	if((state_descriptor == 0) && (sockets.sockets.size() > 0))
	{
		state_descriptor = 1;
		
		// send the beginning commands to all players currently connected.
		serverMsgs.push_back("-2 GO#");
	}
	
	
	// transmit serverMsgs to players
	for(int k=0; k < static_cast<int>(serverMsgs.size()); k++)
		for(map<int, MU_Socket>::iterator i = sockets.sockets.begin(); i != sockets.sockets.end(); i++)
			i->second.write(serverMsgs[k]);
		serverMsgs.clear();
}





void Game::acceptConnections()
{
	// accept any incoming connections
	if(serverSocket.readyToRead() == 1)
	{
		cerr << "looks like someone is connecting :O" << endl;
		serverSocket.accept_connection(sockets);
		
		view.pushMessage("Player CONNECTED.");
		
		
		// if game in progress, inform everyone else of a new connecting player TODO
		MU_Socket& connectingPlayer = sockets.sockets[sockets.nextConnection-1];
		
		// send new player the current simulRules state
		stringstream simulRules_msg;
		simulRules_msg << "-2 SIMUL " << simulRules.currentFrame << " " << simulRules.windowSize << " " <<  simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << "#";
		connectingPlayer.write(simulRules_msg.str());
		
		// send new player the current state of the world:
		for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
		{
			stringstream hero_msg;
			hero_msg << "-2 UNIT " << iter->first << " " << iter->second.angle << " " << iter->second.keyState << " " << iter->second.position.x.number << " " << iter->second.position.y.number << " " << iter->second.position.h.number << " " << iter->second.velocity.x.number << " " << iter->second.velocity.y.number << " " << iter->second.velocity.h.number << "#";
			
			connectingPlayer.write(hero_msg.str());
		}
		
		// send new player current pending orders
		for(int i=0; i<UnitInput.size(); i++)
		{
			stringstream input_msg;
			input_msg << "-4 " << UnitInput[i].frameID << " " << UnitInput[i].plr_id << " " << UnitInput[i].keyState << " " << UnitInput[i].mousex << " " << UnitInput[i].mousey << " " << UnitInput[i].serverCommand << "#";
			connectingPlayer.write(input_msg.str());
		}
		
		// tell the new player what his player ID is.
		stringstream playerID_msg;
		int playerID_val = world.nextUnitID();
		
		playerID_msg << "-1 " << (simulRules.currentFrame + simulRules.windowSize) << " 2 " << playerID_val << "#";
		connectingPlayer.write(playerID_msg.str());
		cerr << "player is expected to assume his role at frame " << (simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize) << endl;
		
		// send the new player info about other players
		for(map<int, PlayerInfo>::iterator iter = Players.begin(); iter != Players.end(); iter++) {
			stringstream playerInfo_msg;
			playerInfo_msg << "2 " << iter->first << " " << iter->second.name << "#";
			connectingPlayer.write(playerInfo_msg.str());
		}

		// send to everyone the ADDHERO msg
		int birth_time = simulRules.currentFrame + simulRules.windowSize;
		
		stringstream createHero_msg;
		connectingPlayer.last_order = birth_time + simulRules.frameSkip * simulRules.windowSize;
		
		createHero_msg << "-1 " << birth_time << " 1 " << playerID_val << "#";
		serverMsgs.push_back(createHero_msg.str());
		cerr << "Hero for player " << playerID_val << " is scheduled for birth at frame " << birth_time << endl;
		
		stringstream clientState_msg;
		clientState_msg << "-2 CLIENT_STATE " << client_state << "#";
		connectingPlayer.write(clientState_msg.str());
		
	}
}


