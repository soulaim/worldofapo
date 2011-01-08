#include "game.h"
#include "logger.h"
#include "worldevent.h"
#include "visualworld.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#include <SDL.h>


// required for obtaining character keys from file. could be located in another file.
#include <fstream>

using namespace std;


Game::Game(World* w):
	world(w)
{
	paused_state = PAUSED;
	myID = NO_ID;
}

void Game::reset()
{
	Players[SERVER_ID].name = "GOD";
	
	clientOrders.clear();
	serverMsgs.clear();
	UnitInput.clear();
	SpawningHeroes.clear();
	Players.clear();
	
	paused_state = PAUSED;
	simulRules = StateInfo();
	
	// need to erase localPlayer data? possible?
	localPlayer = PlayerInfo();
	
	cerr << "Reseting client state." << endl;
	myID = NO_ID;
	
	UnitInput.clear();
	world->terminate();
	world->init();
}

void Game::init()
{
	reset();
	readConfig();
}

void Game::write(int id, const string& msg)
{
	clientSocket.write(id, msg);
}

void Game::readConfig()
{
	load("configs/user.conf");
	
	//ifstream configFile("config.cfg");
	//configFile >> localPlayer.name;
	
	string name = strVals["player\\sname"];
	
	if(name == "")
	{
		char *e = getenv("USER");
		if(e)
			name.assign(e);
		else
			name = "Player5";
		
		ofstream ofs("configs/user.conf");
		ofs << "STRING player\\sname " << name << endl;
	}
	
	localPlayer.name = name;
}

bool Game::internetGameGetHeroes(const string& hostname, map<string, CharacterInfo>& heroes)
{
	myID = NO_ID;
	int port = 12345;
	
	while(!clientSocket.open(SERVER_ID, hostname, port))
	{
		port--;
		if(port < 12320)
		{
			cerr << "superfail :(" << endl;
			return false;
		}
	}
	
	if(!getHeroes(heroes))
		return false;
	
	// need to enter a menu at this point, showing the user which options he has
	// Game does not have access to graphics, so we must return now, and let others handle it.
	return true;
}

void Game::internetGameSelectHero(const string& hero)
{
	stringstream herocommand;
	herocommand << "START " << hero << "#";
	clientSocket.write(SERVER_ID, herocommand.str());
	return;
}

bool Game::getHeroes(map<string, CharacterInfo>& heroes)
{
	set<string> keys = KeyManager::readKeys();
	for(auto i = keys.begin(); i!=keys.end(); i++)
	{
		stringstream ss;
		ss << "OPTION " << (*i) << "#";
		cerr << "sending query for key " << *i << endl;
		clientSocket.write(SERVER_ID, ss.str());

		bool not_finished = true;
		while(not_finished)
		{
			if(check_messages_from_server())
			{
				cerr << "Client connection has died during sign-in process. :(" << endl;
				clientOrders.orders.clear();
				return false;
			}
			
			for(size_t k=0; k<clientOrders.orders.size(); k++)
			{
//				Logger log;
//				log.print("Got handshake message: ---" + clientOrders.orders[k] + "---\n");
				
				not_finished = false;
				string cmd;
				stringstream ss(clientOrders.orders[k]);
				ss >> cmd;
				
				if(cmd == "YES")
				{
					getline(ss, cmd);
					
					// build a character info out of string cmd
					heroes[*i] = CharacterInfo();
					heroes[*i].readDescription(cmd);
				}
				else if(cmd == "NO")
				{
					// no reaction.
				}
			}
			
			clientOrders.orders.clear();
		}
	}
	
	CharacterInfo& ci = heroes["NEW"];
	ci.playerInfo.name = "New\\sCharacter";
	
	clientOrders.orders.clear();
	return true;
}

void Game::endGame()
{
	clientSocket.close(SERVER_ID);
	reset();
}

void Game::set_current_frame_input(int keystate, int x, int y, int mousepress)
{
	int frame = simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize;
	
	if(myID >= 0)
	{
		stringstream inputMsg;
		string msg;

		World::CheckSumType checksum = world->checksum();
		
		
		inputMsg << "1 " << myID << " " << frame << " " << keystate << " " << x << " " << y << " " << mousepress << " " << checksum << "#";
		msg = inputMsg.str();
		clientSocket.write(SERVER_ID, msg);

		//Logger log;
		//log.print("Sent message: +++" + msg + "+++\n");
	}
}

void Game::send_chat_message(const std::string& clientCommand)
{
	stringstream tmp_msg;
	tmp_msg << "3 " << myID << " " << clientCommand << "#";
	clientSocket.write(SERVER_ID, tmp_msg.str());

//	Logger log;
//	log.print("Sent message: +++" + tmp_msg.str() + "+++\n");
}

bool Game::paused() const
{
	return paused_state == PAUSED;
}

bool Game::client_tick_local()
{
	// this is acceptable because the size is guaranteed to be insignificantly small
	sort(UnitInput.begin(), UnitInput.end());
	
	// handle any server commands intended for this frame
	while((UnitInput.back().plr_id == SERVER_ID) && (UnitInput.back().frameID == simulRules.currentFrame))
	{
		Order server_command = UnitInput.back();
		UnitInput.pop_back();
		handleServerMessage(server_command);
	}
	
	if( (simulRules.currentFrame < simulRules.allowedFrame) )
	{
		if( (UnitInput.back().plr_id == SERVER_ID) && (UnitInput.back().frameID != simulRules.currentFrame) )
			cerr << "WARNING: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;
		return true;
	}

	return false;
}


void Game::process_received_game_input()
{
	if(UnitInput.empty())
		return;
	
	sort(UnitInput.begin(), UnitInput.end());
	
	while(UnitInput.back().frameID < simulRules.currentFrame)
	{
		cerr << "WARNING: Client skipped an order for frame " << UnitInput.back().frameID << ", current frame = " << simulRules.currentFrame << endl;
		UnitInput.pop_back();
	}
	
	// update commands of player controlled characters
	while(UnitInput.back().frameID == simulRules.currentFrame)
	{
		Order tmp = UnitInput.back();
		UnitInput.pop_back();
		
		if(tmp.plr_id == SERVER_ID)
		{
			cerr << "WARNING: Someone claims to be server. This should never happen." << endl;
			continue;
		}
		
		auto it = world->units.find(tmp.plr_id);
		
		if(it != world->units.end())
		{
			it->second.updateInput(tmp.keyState, tmp.mousex, tmp.mousey, tmp.mouseButtons);
		}
	}
}

// server messages read from the network
void Game::handleServerMessage(const Order& server_msg)
{
	if(server_msg.serverCommand == 3) // pause!
	{
		paused_state = PAUSED;
		cerr << "Pausing the game at frame " << simulRules.currentFrame << endl;
	}
	else if(server_msg.serverCommand == 10) // This is probably alien spawning.
	{
		world->addUnit(world->nextUnitID(), false);
	}
	else if(server_msg.serverCommand == 11) // change team message
	{
		int unitID     = server_msg.keyState;
		int new_teamID = server_msg.mousex;
		
		Unit& u = world->units.find(unitID)->second;
		u["TEAM"] = new_teamID;
		
		stringstream newViewMessage;
		newViewMessage << "^R" << u.name << " ^Wchanged team ID to ^G" << new_teamID << "!";
		world->add_message(newViewMessage.str());
	}
	else if(server_msg.serverCommand == 15)
	{
		world->addUnit(world->nextUnitID(), false, server_msg.keyState);
	}
	else if(server_msg.serverCommand == 100) // SOME PLAYER HAS DISCONNECTED
	{
		string message = "^R[";
		message.append(Players[server_msg.keyState].name);
		message.append("] has disconnected!");
		world->add_message(message);
		
		world->removeUnit(server_msg.keyState);
		Players.erase(server_msg.keyState);
		simulRules.numPlayers--;
		// BWAHAHAHA...
	}
	else if(server_msg.serverCommand == 1) // ADDHERO message
	{
		string areaName = SpawningHeroes[server_msg.keyState].unit.strVals["AREA"];
		
		if(world->strVals["AREA_NAME"] == areaName)
		{
			world->addUnit(server_msg.keyState);
			simulRules.numPlayers++;
			
			cerr << "Adding a new hero at frame " << simulRules.currentFrame << ", units.size() = " << world->units.size() << ", myID = " << myID << endl;
			
			// the new way
			if(SpawningHeroes[server_msg.keyState].unit.name == "nameless")
			{
				SpawningHeroes[server_msg.keyState].unit.name       = Players[server_msg.keyState].name;
				SpawningHeroes[server_msg.keyState].playerInfo.name = Players[server_msg.keyState].name;
			}
			
			SpawningHeroes[server_msg.keyState].unit.birthTime = world->units[server_msg.keyState].birthTime;
			SpawningHeroes[server_msg.keyState].unit.id = server_msg.keyState;
			
			Unit& unit = world->units.find(server_msg.keyState)->second;
			unit = SpawningHeroes[server_msg.keyState].unit;
			world->intVals["NO_INPUT"] = 10; // NOTE: to prevent de-sync!
			
			Players[server_msg.keyState] = SpawningHeroes[server_msg.keyState].playerInfo;
			SpawningHeroes.erase(server_msg.keyState);
			
			world->add_message("^GHero created!");
			
			sort(UnitInput.begin(), UnitInput.end());
		}
		else
		{
			world->add_message("^GA hero was created into another area!");
		}
	}
	else if(server_msg.serverCommand == 2) // "set playerID" message
	{
		myID = server_msg.keyState; // trololol. nice place to store the info.
		world->add_message("^Ggot playerID!");
		
		cerr << "MYID: " << myID << ", sending my name now: " << localPlayer.name << endl;
		stringstream ss;
		
		if(localPlayer.name == "")
		{
			localPlayer.name = "nameless";
		}
		
		ss << "2 " << myID << " " << localPlayer.name << "#";
		clientSocket.write(SERVER_ID, ss.str());

//		Logger log;
//		log.print("Sent message: +++" + ss.str() + "+++\n");
		
		
		if(world->units.find(myID) != world->units.end())
		{
			WorldEvent event;
			event.type = WorldEvent::CENTER_CAMERA;
			event.actor_id = myID;
			world->add_event(event);
			cerr << "Creating event to bind camera to unit " << myID << "\n";
		}
		else
		{
			cerr << "Failed to bind camera! :(" << endl;
		}
	}
	else if(server_msg.serverCommand == 24) // toggle god-mode
	{
		int id = server_msg.keyState;
		
		auto unit_it = world->units.find(id);
		if(unit_it != world->units.end())
		{
			world->add_message(unit_it->second.name + " toggled god-mode!");
			unit_it->second.intVals["GOD_MODE"] ^= 1;
		}
		
	}
	else if(server_msg.serverCommand == 7) // destroy hero, for area change -message
	{
		int destroy_ID = server_msg.keyState;
		world->removeUnit(destroy_ID);
		
		if(destroy_ID == myID)
		{
			myID = NO_ID;
			world->intVals["NO_INPUT"] = 800000;
			world->terminate();
			UnitInput.clear();
			paused_state = PAUSED;
		}
	}
	else
	{
		cerr << "SERVERMESSAGEFUCK: " << server_msg.serverCommand << endl;
	}
}



// client messages read from the network
void Game::processClientMsgs()
{
	for(size_t i = 0; i < clientOrders.orders.size(); ++i)
	{
		if(clientOrders.orders[i] == "")
			continue;

//		Logger log;
//		log.print("Got message: ---" + clientOrders.orders[i] + "---\n");

		stringstream ss(clientOrders.orders[i]);
		
		int order_type;
		ss >> order_type;
		
		if(order_type == 1) // ordinary input order
		{
			if(paused_state == PAUSED)
			{
				cerr << "RECEIVED GAME COMMAND ALTHOUGH IM IN PAUSE MODE! NOT GOING TO PROCESS IT" << endl;
			}
			else
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
		}
		else if(order_type == 3) // chat message
		{
			cerr << "ORDER: " << clientOrders.orders[i] << endl;
			
			int plrID;
			string line;
			
			ss >> plrID;
			getline(ss, line);
			
			stringstream chatMsg;
			chatMsg << "^y<" << Players[plrID].name << "> ^w" << line;
			world->add_message(chatMsg.str());
		}

		else if(order_type == 2) // playerInfo message
		{
			cerr << "ORDER: \"" << clientOrders.orders[i] << "\"" << endl;
			
			if(clientOrders.orders[i].size() < 4) // less than four long, it can't hold all the necessary data.
			{
				cerr << "This message seems to be full of shit :/ I'll just ignore it or something.." << endl;
			}
			else
			{
				cerr << "Got playerInfo message!" << endl;
				int plrID, kills, deaths;
				string name;
				ss >> plrID >> kills >> deaths;
				
				ss.ignore(); // eat away the extra space delimiter
				getline(ss, name);
				
				Players[plrID].name = name;
				Players[plrID].kills = kills;
				Players[plrID].deaths = deaths;
				
				cerr << plrID << " " << name << " (" << kills << "/" << deaths << ")" << endl;
				
				stringstream message;
				message << "^g" << Players[plrID].name << "^r has connected!" << endl;
				world->add_message(message.str());
				
				// set unit's name to match the players
				if(world->units.find(plrID) == world->units.end())
				{
					cerr << "GOT playerInfo MESSAGE TOO SOON :G Will just place the name at hero birth." << endl;
				}
				else
				{
					cerr << "Assigning player identity (name) to corresponding unit." << endl;
					world->units.find(plrID)->second.name = Players[plrID].name;
				}
			}
		}
		
		
		else if(order_type == -1) // A COMMAND message from GOD (server)
		{
			Order tmp_order;
			tmp_order.plr_id = SERVER_ID;
			ss >> tmp_order.frameID;
			ss >> tmp_order.serverCommand;
			ss >> tmp_order.keyState;
			ss >> tmp_order.mousex >> tmp_order.mousey; // use these as more storage space
			UnitInput.push_back(tmp_order);
		}
		
		
		else if(order_type == -2) // instant reaction message from GOD
		{
			string cmd;
			ss >> cmd;
			
			// allow messages handled first, since they are coming in all the time
			if(cmd == "ALLOW")
			{
				int frame;
				ss >> frame;
				
				// cerr << "SERVER ALLOWED SIMULATION UP TO FRAME: " << frame << endl;
				simulRules.allowedFrame = frame;
			}
			else if(cmd == "GO")
			{
				cerr << "Setting client state to UNPAUSED" << endl;
				paused_state = GO;
			}
			
			else if(cmd == "CHANGE_PROPERTY")
			{
				string line;
				getline(ss, line);
				
				// not the cleanest way to do this, but i really want to react to this message in LocalPlayer
				WorldEvent event;
				event.type = WorldEvent::SET_LOCAL_PROPERTY;
				event.cmd = line;
				
				world->add_event(event);
			}
			else if(cmd == "CLEAR_WORLD_PROPERTIES")
			{
				world->intVals.clear();
				world->strVals.clear();
			}
			else if(cmd == "WORLD_PROPERTY")
			{
				char type;
				ss >> type;
				
				if(type == 'S')
				{
					string name;
					string value;
					ss >> name >> value;
					world->strVals[name] = value;
				}
				else if(type == 'I')
				{
					string name;
					int value;
					ss >> name >> value;
					world->intVals[name] = value;
				}
				else
				{
					throw std::logic_error("WORLD_PROPERTY MESSAGE BROKEN");
				}
			}
			
			
			else if(cmd == "GIVE_NAME") // request player name message
			{
				// hmm
			}
			else if(cmd == "CHAR_KEY") // I just received a character key :O I better store it..
			{
				cerr << "GOT A CHARACTER KEY CODE. SAVING IT TO FILE." << endl;
				string characterKey; ss >> characterKey;
				KeyManager::saveKey(characterKey);
			}
			else if(cmd == "UNIT") // unit copy message
			{
				
				cerr << "Copy unit: " << clientOrders.orders[i] << endl;
				
				int unitID;
				ss >> unitID;
				
				Logger log;
				log << "ORDER: create unit " << unitID << "\n";
				
				
				// ALERT TODO: THIS IS BUBBLEGUM TO CREATE THE CORRECT MODEL FOR THE UNIT
				/*
				Unit tmp_unit;
				tmp_unit.init();
				tmp_unit.handleCopyOrder(ss);
				
				if(tmp_unit.human())
				{
					world->addUnit(unitID);
				}
				else
				{
					world->addUnit(unitID, false);
				}
				
				world->units.find(unitID)->second = tmp_unit;
				*/
				// END OF ALERT TODO
				
				world->addUnit(unitID);
				Unit& unit = world->units.find(unitID)->second;
				unit.handleCopyOrder(ss);
				
				VisualWorld::ModelType type = world->visualworld->getModelType(unit.name);
				float scale = unit.scale.getFloat();
				world->visualworld->createModel(unitID, unit.position, type, scale);
				
				
				cerr << "COPY OF A UNIT: " << unitID << " / " << unit.id << endl;
			}
			else if(cmd == "ITEM")
			{
				int itemID;
				ss >> itemID;
				
				Logger log;
				log << "ORDER: create item " << itemID << "\n";
				
				Location dummy;
				world->addItem(dummy, dummy, itemID);
				auto item_it = world->items.find(itemID);
				item_it->second.handleCopyOrder(ss);
				world->visualworld->createModel(itemID, item_it->second.position, VisualWorld::ModelType(item_it->second.intVals["MODEL_TYPE"]), 1.0f);
			}
			else if(cmd == "PROJECTILE")
			{
				int id;
				size_t prototype_model;
				ss >> id >> prototype_model;
				Location dummy;
				world->addProjectile(dummy, id, prototype_model);
				world->projectiles[id].handleCopyOrder(ss);
			}
			else if(cmd == "NEXT_UNIT_ID")
			{
				int id = -1;
				ss >> id;
				world->setNextUnitID(id);
			}
			else if(cmd == "SIMUL")
			{
				cerr << "Set simulRules to instructed state" << endl;
				ss >> simulRules.currentFrame >> simulRules.windowSize >> simulRules.frameSkip >> simulRules.numPlayers >> simulRules.allowedFrame;
				cerr << simulRules.currentFrame << " " << simulRules.windowSize << " " << simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << endl;
			}
			else if(cmd == "CLIENT_STATE")
			{
				int nopause = 0;
				ss >> nopause;
				paused_state = (nopause ? GO : PAUSED);
				
				if(paused_state == GO)
				{
					sort(UnitInput.begin(), UnitInput.end());
					cerr << "Resuming game simulation!" << endl;
				}
				else
					cerr << "Game simulation set on pause, server orders so." << endl;
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
			else if(cmd == "WORLD_GEN_PARAM")
			{
				
				meta_events.push_back(HasProperties());
				HasProperties& event = meta_events.back();
				
				int param;
				string area_name;
				
				ss >> param >> area_name;
				
				event.strVals["EVENT_TYPE"] = "WORLD_GEN_PARAM";
				event.intVals["WORLD_GEN_SEED"] = param;
				event.strVals["WORLD_NAME"] = area_name;
				
				// set local simulation on pause
				cerr << "Pausing game simulation.." << endl;
				paused_state = PAUSED;
				
			}
			else
			{
				cerr << "INSTANT ORDER FUCK!" << endl;
			}
			
		}
		
		else if(order_type == -4) // copy of an existing order
		{
			cerr << "ORDER: " << clientOrders.orders[i] << endl;
			
			cerr << "Got a copy of an old message" << endl;
			Order tmp_order;
			tmp_order.handleCopyOrder(ss);
			UnitInput.push_back(tmp_order);
		}
		else
		{
			cerr << "HOLY FUCK! I DONT UNDERSTAND SHIT :G \"" << clientOrders.orders[i] << "\"" << endl;
		}
	}
	clientOrders.orders.clear();
}


bool Game::check_messages_from_server()
{
	clientSocket.tick();
	bool stop = false;
	if(!clientSocket.alive(SERVER_ID))
	{
		clientSocket.close(SERVER_ID);
		cerr << "Client connection has died. :(" << endl;
		stop = true;
	}
	else
	{
		vector<string>& msgs = clientSocket.read(SERVER_ID);
		for(size_t k = 0; k < msgs.size(); ++k)
		{
			clientOrders.insert(msgs[k] + "#"); // TODO: messages are now parsed twice: once in sockethandler and once in clientOrders.
		}
		msgs.clear();
	}
	return stop;
}


void Game::TICK()
{
	// run simulation for one WorldFrame
	world->worldTick(simulRules.currentFrame);
	simulRules.currentFrame++;
}

