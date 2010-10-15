
#include "dedicated.h"


string generateKey()
{
	int val1 = rand();
	
	char c = 0;
	string key;
	
	// length 15 string.
	key = "               ";
	
	for(int i=0; i<15; i++)
	{
		c = '[';
		while(c < 'a' && c > 'Z')
			c = 'A' + ( (++val1 * 71) % ('z'-'A') );
		key[i] = c;
	}
	
	return key;
}



void DedicatedServer::handleSignInMessage(int playerID_val, string order)
{
	stringstream ss(order);
	
	string cmd;
	ss >> cmd;
	
	cerr << "GOT SIGN-IN MSG: " << order << endl;
	
	if(cmd == "OPTION") // client wants to know more about a particular option. send something back.
	{
		ss >> cmd;
		if(dormantPlayers.find(cmd) == dormantPlayers.end())
		{
			cerr << "Could not find the player" << endl;
			
			sockets.sockets[playerID_val].write("NO#");
		}
		else
		{
			cerr << "FOUND THE PLAYER :OO" << endl;
			
			string ans = "YES ";
			ans.append( dormantPlayers[cmd].getDescription() );
			ans.append("#");
			
			sockets.sockets[playerID_val].write(ans);
		}
	}
	else if(cmd == "START")
	{
		ss >> cmd;
		playerStartingChoice(playerID_val, cmd);
	}
}


void DedicatedServer::playerStartingChoice(int playerID_val, string choice)
{
	
	Players[playerID_val].connectionState = 1; // game on!
	simulRules.numPlayers++;
	
	MU_Socket& connectingPlayer = sockets.sockets[playerID_val];
	
	if(choice == "NEW" || (dormantPlayers.find(choice) == dormantPlayers.end()))
	{
		// Set the new character's "password"
		Players[playerID_val].key = generateKey();
		cerr << "Starting a new new player profile with key: " << Players[playerID_val].key << endl;
		
		// transmit player key to client
		stringstream characterKey_msg;
		characterKey_msg << "-2 CHAR_KEY " << Players[playerID_val].key << "#";
		connectingPlayer.write(characterKey_msg.str());
	}
	else
	{
		cerr << "resurrecting a previous profile with key: " << choice << " by name: " << dormantPlayers[choice].name << endl;
		Players[playerID_val] = dormantPlayers[choice];
	}
	
	cerr << "Sending a copy of the world" << endl;
	sendWorldCopy("test_area", playerID_val);
	
	
	cerr << "Sending PLAYER ID" << endl;
	// tell the new player what his player ID is.
	stringstream playerID_msg;
	playerID_msg << "-1 " << (simulRules.currentFrame + simulRules.windowSize) << " 2 " << playerID_val << "#";
	connectingPlayer.write(playerID_msg.str());
	
	
	cerr << "SENDING OTHER PLAYERS" << endl;
	// send the new player some generic info about other players
	for(map<int, PlayerInfo>::iterator iter = Players.begin(); iter != Players.end(); iter++)
	{
		cerr << "sending info!" << endl;
		stringstream playerInfo_msg;
		string clientName = iter->second.name;
		if(clientName == "")
			clientName = "Unknown Player";
		playerInfo_msg << "2 " << iter->first << " " << iter->second.kills << " " << iter->second.deaths << " " << clientName << "#";
		connectingPlayer.write(playerInfo_msg.str());
	}
	
	cerr << "SENDING ADDHERO" << endl;
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




