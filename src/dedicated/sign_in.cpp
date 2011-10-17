
#include "dedicated/dedicated.h"

using namespace Network;

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
			// could not find the player
			socketWrite(sockets, playerID_val, "NO#");
		}
		else
		{
			// found player
			std::string ans = "YES ";

			// put player info into the message
			ans.append( dormantPlayers[cmd].getDescription() );
			ans.append("#");

			socketWrite(sockets, playerID_val, ans);
		}
	}
	else if(cmd == "START")
	{
		ss >> cmd;
		playerStartingChoice(playerID_val, cmd);
	}
	else if(cmd == "ADMIN")
	{
		// ...

	}
	else
	{
		std::cerr << "WARNING: Sign-in command \"" << cmd << "\" was unrecognized." << std::endl;
	}
}

void DedicatedServer::playerStartingChoice(int playerID_val, std::string choice)
{
	// TODO: put this to the other place? not sure how to do that though.
	simulRules.numPlayers++;

	if(choice == "NEW" || (dormantPlayers.find(choice) == dormantPlayers.end()))
	{
		// Set the new character's "password"
		Players[playerID_val].key = generateKey();
		choice = Players[playerID_val].key;

		std::cerr << "Starting a new new player profile with key: " << Players[playerID_val].key << std::endl;

		// transmit player key to client
		std::stringstream characterKey_msg;
		characterKey_msg << "-2 CHAR_KEY " << Players[playerID_val].key << "#";

		socketWrite(sockets, playerID_val, characterKey_msg.str());
	}
	else
	{
		std::cerr << "resurrecting a previous profile with key: " << choice << " by name: " << dormantPlayers[choice].playerInfo.name << std::endl;
		Players[playerID_val] = dormantPlayers[choice].playerInfo;
	}

	if(dormantPlayers[choice].unit.name == "")
	{
		std::string areaName = "default_area";
		World& world = areas.find(areaName)->second;

		// TODO: how about calling some actual unit init function?
		dormantPlayers[choice].unit.init();
		dormantPlayers[choice].unit.model_type = VisualWorld::ModelType::PLAYER_MODEL;
		dormantPlayers[choice].unit["TEAM"] = 0;

		dormantPlayers[choice].unit.position = world.lvl.getStartLocation();
		dormantPlayers[choice].unit.strVals["AREA"] = areaName;
		dormantPlayers[choice].playerInfo.key = Players[playerID_val].key;
	}

	changeArea(playerID_val);
	return;
}



void DedicatedServer::changeArea(int playerID_val)
{
	std::string char_key = Players[playerID_val].key;
	std::string areaName = dormantPlayers[char_key].unit.strVals["AREA"];

	Players[playerID_val].connectionState = ConnectionState::WAIT_WORLD_GEN; // waiting for client world gen to finish
	dormantPlayers[char_key].playerInfo.connectionState = ConnectionState::WAIT_WORLD_GEN; // waiting for level gen to finish.

	std::cerr << "Sending world parameters" << std::endl;
	sendAreaParameters(areaName, playerID_val);
	std::cerr << "Waiting for client world gen to finish before sending game content" << std::endl;
	return;
}



void DedicatedServer::sendWorldContent(int playerID_val)
{
	std::string char_key = Players[playerID_val].key;
	std::string areaName = dormantPlayers[char_key].unit.strVals["AREA"];

	// set player as active
	Players[playerID_val].connectionState = ConnectionState::GAMEPLAY;
	dormantPlayers[char_key].playerInfo.connectionState = ConnectionState::GAMEPLAY;


	// NOT SURE IF THIS IS OF ANY USE
	int orders_destroyed = 0;
	for(size_t i = 0; i < UnitInput.size(); i++)
	{
		if(UnitInput[i].plr_id == playerID_val)
		{
			orders_destroyed++;

			UnitInput[i] = UnitInput.back();
			UnitInput.pop_back();
			--i;
		}
	}

	std::cerr << "Destroyed " << orders_destroyed << " orders prior to allowing connection." << std::endl;
	sort(UnitInput.begin(), UnitInput.end());
	// END SEGMENT


	std::cerr << "Sending a copy of the world" << std::endl;
	sendWorldCopy(areaName, playerID_val);

	std::cerr << "Sending PLAYER ID" << std::endl;
	std::stringstream playerID_msg;

	playerID_msg << "-1 " << (simulRules.currentFrame + simulRules.windowSize) << " 2 " << playerID_val << "#";
	socketWrite(sockets, playerID_val, playerID_msg.str());


	std::cerr << "SENDING OTHER PLAYERS" << std::endl;
	// send the new player some generic info about other players
	for(auto iter = Players.begin(); iter != Players.end(); iter++)
	{
		std::cerr << "sending info!" << std::endl;
		std::stringstream playerInfo_msg;
		std::string clientName = iter->second.name;
		if(clientName == "")
			clientName = "nameless";
		playerInfo_msg << "2 " << iter->first << " " << iter->second.kills << " " << iter->second.deaths << " " << clientName << "#";

		socketWrite(sockets, playerID_val, playerInfo_msg.str());
	}


	// send to everyone the character info of the spawning unit / player.
	Players[playerID_val] = dormantPlayers[char_key].playerInfo;

	std::string character_info_str = dormantPlayers[char_key].getDescription();
	std::stringstream character_info_msg;

	// ALERT: sending the character key to everyone!
	character_info_msg << "-2 CHAR_INFO " << playerID_val << " " << character_info_str;
	serverMsgs.push_back(character_info_msg.str());


	// send to everyone the ADDHERO msg
	std::cerr << "SENDING ADDHERO" << std::endl;
	int birth_time = simulRules.currentFrame + simulRules.windowSize; // this could be just currentFrame + 1 ..

	std::stringstream createHero_msg;
	// Players[playerID_val].last_order = birth_time; // + simulRules.frameSkip * simulRules.windowSize;

	createHero_msg << "-1 " << birth_time << " 1 " << playerID_val << "#";
	serverMsgs.push_back(createHero_msg.str());
	std::cerr << "Hero for player " << playerID_val << " is scheduled for birth at frame " << birth_time << std::endl;

	// send id generator state
	std::stringstream nextUnit_msg;
	nextUnit_msg << "-2 NEXT_UNIT_ID " << areas.find(areaName)->second.currentUnitID() << "#";
	socketWrite(sockets, playerID_val, nextUnit_msg.str());


	// Now that all game info has been sent, can send messages to allow the client to start his own simulation.
	std::stringstream simulRules_msg;
	simulRules_msg << "-2 SIMUL " << simulRules.currentFrame << " " << simulRules.windowSize << " " <<  simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << "#";

	socketWrite(sockets, playerID_val, simulRules_msg.str());

	// go!
	std::stringstream clientState_msg;
	clientState_msg << "-2 CLIENT_STATE " << 1 << "#";

	socketWrite(sockets, playerID_val, clientState_msg.str());
}

