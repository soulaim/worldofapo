
#include "game.h"

// for debugging
#include "logger.h"

#include <sstream>
#include <algorithm>

using namespace std;


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
		string viewMessage_str = "^R[";
		viewMessage_str.append(Players[server_msg.keyState].name);
		viewMessage_str.append("] has disconnected!");
		view->pushMessage(viewMessage_str);
		
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
		
		view->pushMessage("^GHero created!");
		
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
		view->pushMessage("^Ggot playerID!");
		
		
		
		cerr << "Sending my name now: " << localPlayer.name << endl;
		stringstream ss;
		
		if(localPlayer.name == "")
		{
			localPlayer.name = "Unknown player";
		}
		
		ss << "2 " << myID << " 0 0 " << localPlayer.name << "#";
		clientSocket.write(ss.str());
		
		
		
		
		
		if(world->units.find(myID) != world->units.end())
		{
			cerr << "Binding camera to player " << myID << "\n";
			view->bindCamera(&world->units[myID]);
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
			view->pushMessage(chatMsg.str());
		}
		
		else if(order_type == 2) // playerInfo message
		{
			cerr << "ORDER: " << clientOrders.orders[i] << endl;
			
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
			
			stringstream ss_viewMsg;
			ss_viewMsg << "^g" << Players[plrID].name << "^r has connected!" << endl;
			view->pushMessage(ss_viewMsg.str());
			
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
				cerr << "Setting client state to 1" << endl;
				paused_state = GO;
				fps_world.reset();
				fps_world.setStartTime( SDL_GetTicks() );
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
				if(nopause == 0)
				{
					paused_state = PAUSED;
				}
				else
				{
					paused_state = GO;
				}
				fps_world.reset();
				fps_world.setStartTime( SDL_GetTicks() );
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
			cerr << "HOLY FUCK! I DONT UNDERSTAND SHIT :G" << endl;
		}
	}
	
	clientOrders.orders.clear();
}


void Game::check_messages_from_server()
{
	if(clientSocket.readyToRead() == 1)
	{
		string msg = clientSocket.read();
		
		if(msg.size() == 0)
		{
			clientSocket.closeConnection();
			cerr << "Client connection has died. :(" << endl;
			state = "menu";
		}
		
		clientOrders.insert(msg); // give it to orderhandler to be parsed down to single commands
		processClientMsgs();
	}
}

