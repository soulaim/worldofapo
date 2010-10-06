
#include "game.h"

using namespace std;

void Game::serverSendMonsterSpawn()
{
	stringstream tmp_msg;
	tmp_msg << "-1 " << (serverAllow+1) << " 10" << "#";
	for(map<int, MU_Socket>::iterator target = sockets.sockets.begin(); target != sockets.sockets.end(); target++)
		target->second.write(tmp_msg.str());
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
	
	
	
	if(minAllowed < 5)
		minAllowed = 5;
	
	if( (minAllowed != ~0) && (minAllowed > serverAllow) )
	{
		stringstream allowSimulation_msg;
		allowSimulation_msg << "-2 ALLOW " << minAllowed << "#";
		
		// cerr << "minAllowed: " << minAllowed << ", serverAllow: " << serverAllow << endl;
		
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
		
		// if game in progress, inform everyone else of a new connecting player TODO
		MU_Socket& connectingPlayer = sockets.sockets[sockets.nextConnection-1];
		
		// send new player the current simulRules state
		stringstream simulRules_msg;
		simulRules_msg << "-2 SIMUL " << simulRules.currentFrame << " " << simulRules.windowSize << " " <<  simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << "#";
		connectingPlayer.write(simulRules_msg.str());
		
		// send new player the current state of the world:
		for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
			connectingPlayer.write(iter->second.copyOrder(iter->first));
		
		// send new player the current state of the world:
		for(map<int, Projectile>::iterator iter = world.projectiles.begin(); iter != world.projectiles.end(); iter++)
			connectingPlayer.write(iter->second.copyOrder(iter->first));
		
		// send new player current pending orders
		for(int i=0; i<UnitInput.size(); i++)
			connectingPlayer.write(UnitInput[i].copyOrder());
		
		// tell the new player what his player ID is.
		stringstream playerID_msg;
		int playerID_val = world.nextPlayerID();
		
		playerID_msg << "-1 " << (simulRules.currentFrame + simulRules.windowSize) << " 2 " << playerID_val << "#";
		connectingPlayer.write(playerID_msg.str());
		cerr << "player is expected to assume his role at frame " << (simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize) << endl;
		
		// send the new player some generic info about other players
		for(map<int, PlayerInfo>::iterator iter = Players.begin(); iter != Players.end(); iter++)
		{
			cerr << "sending info!" << endl;
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
		
		stringstream nextUnit_msg;
		nextUnit_msg << "-2 NEXT_UNIT_ID " << world._unitID_next_unit << "#";
		connectingPlayer.write(nextUnit_msg.str());
		
		stringstream clientState_msg;
		clientState_msg << "-2 CLIENT_STATE " << client_state << "#";
		connectingPlayer.write(clientState_msg.str());
		
	}
}


