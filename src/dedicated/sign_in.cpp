
#include "dedicated.h"


std::string generateKey()
{
	char c = 0;
	std::string key;
	
	// length 15 string.
	key = "               ";
	
	for(int i=0; i<15; i++)
	{
		c = '[';
		while( !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) )
		{
			int val1 = rand();
			c = 'A' + ( (val1 * 71) % ('z'-'A') );
		}
		key[i] = c;
	}
	
	return key;
}



void DedicatedServer::handleSignInMessage(int playerID_val, std::string order)
{
	std::stringstream ss(order);
	
	std::string cmd;
	ss >> cmd;
	
	std::cerr << "GOT SIGN-IN MSG: " << order << std::endl;
	
	if(cmd == "OPTION") // client wants to know more about a particular option. send something back.
	{
		ss >> cmd;
		if(dormantPlayers.find(cmd) == dormantPlayers.end())
		{
			std::cerr << "Could not find the player" << std::endl;
			
			sockets.write(playerID_val, "NO#");
		}
		else
		{
			std::cerr << "FOUND THE PLAYER :OO" << std::endl;
			
			std::string ans = "YES ";
			ans.append( dormantPlayers[cmd].getDescription() );
			ans.append("#");
			
			sockets.write(playerID_val, ans);
		}
	}
	else if(cmd == "START")
	{
		ss >> cmd;
		playerStartingChoice(playerID_val, cmd);
	}
}


void DedicatedServer::playerStartingChoice(int playerID_val, std::string choice)
{
	
	Players[playerID_val].connectionState = 1; // game on!
	simulRules.numPlayers++;
	
	if(choice == "NEW" || (dormantPlayers.find(choice) == dormantPlayers.end()))
	{
		// Set the new character's "password"
		Players[playerID_val].key = generateKey();
		std::cerr << "Starting a new new player profile with key: " << Players[playerID_val].key << std::endl;
		
		// transmit player key to client
		std::stringstream characterKey_msg;
		characterKey_msg << "-2 CHAR_KEY " << Players[playerID_val].key << "#";
		sockets.write(playerID_val, characterKey_msg.str());
	}
	else
	{
		std::cerr << "resurrecting a previous profile with key: " << choice << " by name: " << dormantPlayers[choice].name << std::endl;
		Players[playerID_val] = dormantPlayers[choice];
	}
	
	std::cerr << "Sending a copy of the world" << std::endl;
	sendWorldCopy("test_area", playerID_val);
	
	
	std::cerr << "Sending PLAYER ID" << std::endl;
	// tell the new player what his player ID is.
	std::stringstream playerID_msg;
	playerID_msg << "-1 " << (simulRules.currentFrame + simulRules.windowSize) << " 2 " << playerID_val << "#";
	sockets.write(playerID_val, playerID_msg.str());
	
	
	std::cerr << "SENDING OTHER PLAYERS" << std::endl;
	// send the new player some generic info about other players
	for(auto iter = Players.begin(); iter != Players.end(); iter++)
	{
		std::cerr << "sending info!" << std::endl;
		std::stringstream playerInfo_msg;
		std::string clientName = iter->second.name;
		if(clientName == "")
			clientName = "Unknown Player";
		playerInfo_msg << "2 " << iter->first << " " << iter->second.kills << " " << iter->second.deaths << " " << clientName << "#";
		sockets.write(playerID_val, playerInfo_msg.str());
	}
	
	std::cerr << "SENDING ADDHERO" << std::endl;
	// send to everyone the ADDHERO msg
	int birth_time = simulRules.currentFrame + simulRules.windowSize;
	
	std::stringstream createHero_msg;
	Players[playerID_val].last_order = birth_time + simulRules.frameSkip * simulRules.windowSize;
	
	createHero_msg << "-1 " << birth_time << " 1 " << playerID_val << "#";
	serverMsgs.push_back(createHero_msg.str());
	std::cerr << "Hero for player " << playerID_val << " is scheduled for birth at frame " << birth_time << std::endl;
	
	std::stringstream nextUnit_msg;
	nextUnit_msg << "-2 NEXT_UNIT_ID " << world.currentUnitID() << "#";
	sockets.write(playerID_val, nextUnit_msg.str());
	
	
	
	// Now that all game info has been sent, can send messages to allow the client to start his own simulation.
	std::stringstream simulRules_msg;
	simulRules_msg << "-2 SIMUL " << simulRules.currentFrame << " " << simulRules.windowSize << " " <<  simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << "#";
	sockets.write(playerID_val, simulRules_msg.str());
	
	// go!
	std::stringstream clientState_msg;
	clientState_msg << "-2 CLIENT_STATE " << 1 << "#";
	sockets.write(playerID_val, clientState_msg.str());
}

