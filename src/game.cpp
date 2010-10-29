#include "game.h"
#include "logger.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL/SDL.h>

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
	
	cerr << "Reseting client state." << endl;
	myID = NO_ID;
	
	UnitInput.clear();
	world->terminate();
}

void Game::init()
{
	reset();
	readConfig();
}

void Game::readConfig()
{
	//ifstream configFile("config.cfg");
	//configFile >> localPlayer.name;
	string name;
	char *e = getenv("USER");
	if(e)
	  name.assign(e);
	else
	  name = "failname";
	localPlayer.name = name;
}

bool Game::joinInternetGame(const string& hostname)
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
	

	map<string, string> heroes;
	if (!getHeroes(heroes))
		return false;

	// menu
	string hero = temp_menu_which_should_be_removed(heroes);
	
	stringstream herocommand;
	herocommand << "START " << hero << "#";
	clientSocket.write(SERVER_ID, herocommand.str());
	cerr << "Starting with " << hero << "." << endl;
	return true;
}

bool Game::getHeroes(map<string, string>& heroes)
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
					heroes[*i] = cmd;
					clientOrders.orders.clear();
				}
				else if(cmd == "NO")
					clientOrders.orders.clear();
			}
		}
	}

	heroes["NEW"] = "New Character";

	clientOrders.orders.clear();

	return true;
}

string Game::temp_menu_which_should_be_removed(const map<string, string> heroes)
{
	if (heroes.size() == 1)
		return heroes.begin()->first;
	cerr << "Your heroes" << endl;
	for(map<string, string>::const_iterator iter = heroes.begin(); iter != heroes.end(); iter++)
	{
		cerr << "Hero code '" << iter->first << "': " << iter->second << endl;
	}
	string code;
	while (1)
	{
		cerr << "Enter hero code" << endl;
		cin >> code;
		if (heroes.find(code) != heroes.end())
		{
			return code;
		}
		else
			cerr << "Not valid hero code" << endl;
	}
}


void Game::endGame()
{
	clientSocket.close(SERVER_ID);
	
	// TODO: release local game resources?
}

void Game::set_current_frame_input(int keystate, int x, int y, int mousepress)
{
	int frame = simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize;
	
	if(myID >= 0)
	{
		stringstream inputMsg;
		string msg;

		unsigned long checksum = world->checksum();
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
			cerr << "ERROR: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;

		return true;
	}

	return false;
}


void Game::process_received_game_input()
{
//	Logger log;
	assert(!UnitInput.empty() && "FUUUUUUUUUUU");
	// update commands of player controlled characters
	while(UnitInput.back().frameID == simulRules.currentFrame)
	{
		Order tmp = UnitInput.back();
		UnitInput.pop_back();
		
		// log all processed game data affecting commands in the order of processing
//		log.print(tmp.copyOrder() + "\n");
		
		if(tmp.plr_id == SERVER_ID)
		{
			cerr << "MOTHERFUCKER FUCKING FUCK YOU MAN?= JUST FUCK YOU!!" << endl;
			break;
		}
		
		world->units[tmp.plr_id].updateInput(tmp.keyState, tmp.mousex, tmp.mousey, tmp.mouseButtons);
	}
	
//	log.print("\n");
}

// server messages read from the network
void Game::handleServerMessage(const Order& server_msg)
{
	if(server_msg.serverCommand == 3) // pause!
	{
		paused_state = PAUSED;
		cerr << "Pausing the game at frame " << simulRules.currentFrame << endl;
	}
	else if(server_msg.serverCommand == 10)
	{
		world->addUnit(world->nextUnitID(), false);
	}
	
	else if(server_msg.serverCommand == 100) // SOME PLAYER HAS DISCONNECTED
	{
		string message = "^R[";
		message.append(Players[server_msg.keyState].name);
		message.append("] has disconnected!");
		world->add_message(message);
		
		world->units.erase(server_msg.keyState);
		world->models.erase(server_msg.keyState);
		Players.erase(server_msg.keyState);
		simulRules.numPlayers--;
		// BWAHAHAHA...
	}
	else if(server_msg.serverCommand == 1) // ADDHERO message
	{
		world->addUnit(server_msg.keyState);
		simulRules.numPlayers++;
		cerr << "Adding a new hero at frame " << simulRules.currentFrame << ", units.size() = " << world->units.size() << ", myID = " << myID << endl;
		
		// just to make sure.
		world->units[server_msg.keyState].name = Players[server_msg.keyState].name;
		
		world->add_message("^GHero created!");
		
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
		
	}
	else if(server_msg.serverCommand == 2) // "set playerID" message
	{
		myID = server_msg.keyState; // trololol. nice place to store the info.
		world->add_message("^Ggot playerID!");
		
		cerr << "Sending my name now: " << localPlayer.name << endl;
		stringstream ss;
		
		if(localPlayer.name == "")
		{
			localPlayer.name = "Unknown player";
		}
		
		ss << "2 " << myID << " " << localPlayer.name << "#";
		clientSocket.write(SERVER_ID, ss.str());

//		Logger log;
//		log.print("Sent message: +++" + ss.str() + "+++\n");
		
		
		if(world->units.find(myID) != world->units.end())
		{
			WorldEvent event;
			event.type = World::CENTER_CAMERA;
			event.actor_id = myID;
			world->add_event(event);
			cerr << "Creating event to bind camera to unit " << myID << "\n";
		}
		else
		{
			cerr << "Failed to bind camera! :(" << endl;
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
			Order tmp_order;
			ss >> tmp_order.plr_id;
			ss >> tmp_order.frameID;
			ss >> tmp_order.keyState;
			ss >> tmp_order.mousex >> tmp_order.mousey;
			ss >> tmp_order.mouseButtons;
			ss >> tmp_order.checksum;
			
			UnitInput.push_back(tmp_order);
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
					world->units[plrID].name = Players[plrID].name;
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
			UnitInput.push_back(tmp_order);
		}
		
		
		else if(order_type == -2) // instant reaction message from GOD
		{
			string cmd;
			ss >> cmd;
			if(cmd == "GO")
			{
				cerr << "Setting client state to UNPAUSED" << endl;
				paused_state = GO;
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
				cerr << "Creating a new unit as per instructions" << endl;
				int unitID;
				ss >> unitID;
				world->addUnit(unitID);
				world->units[unitID].handleCopyOrder(ss);
			}
			else if(cmd == "PROJECTILE")
			{
				int id; ss >> id;
				Location paska;
				world->addProjectile(paska, id);
				world->projectiles[id].handleCopyOrder(ss);
			}
			else if(cmd == "NEXT_UNIT_ID")
			{
				ss >> world->_unitID_next_unit;
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
			ss >> tmp_order.frameID >> tmp_order.plr_id >> tmp_order.keyState >> tmp_order.mousex >> tmp_order.mousey >> tmp_order.serverCommand >> tmp_order.mouseButtons;
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

