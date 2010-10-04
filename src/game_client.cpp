
#include "game.h"

#include <sstream>
#include <algorithm>

using namespace std;



void Game::handleServerMessage(const Order& server_msg)
{
	if(server_msg.serverCommand == 3) // pause!
	{
		client_state = 0;
		cerr << "Pausing the game at frame " << simulRules.currentFrame << endl;
	}
	
	
	else if(server_msg.serverCommand == 100) // SOME PLAYER HAS DISCONNECTED
	{
		cerr << "Player #" << server_msg.keyState << " has disconnected :o Erasing everything related to him!" << endl;
		
		view.pushMessage("Player disconnected.");
		
		world.units.erase(server_msg.keyState);
		world.models.erase(server_msg.keyState);
		simulRules.numPlayers--;
		// BWAHAHAHA...
		
	}
	else if(server_msg.serverCommand == 1) // ADDHERO message
	{
		world.addUnit(server_msg.keyState);
		simulRules.numPlayers++;
		cerr << "Adding a new hero at frame " << simulRules.currentFrame << ", units.size() = " << world.units.size() << ", myID = " << myID << endl;
		view.pushMessage("Hero created!");
		
		cerr << "Creating dummy input for new hero." << endl;
		
		// WE MUST CREATE DUMMY INPUT FOR ALL PLAYERS FOR THE FIRST windowSize frames!
		for(int frame = 0; frame < simulRules.windowSize * simulRules.frameSkip; frame++)
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
		cerr << "Setting local playerID at frame " << simulRules.currentFrame << " to value " << myID << endl;
		view.pushMessage("got playerID!");
		
		if(world.units.find(myID) != world.units.end())
		{
			cerr << "Binding camera to player " << myID << "\n";
			view.bindCamera(&world.units[myID]);
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




void Game::processClientMsgs()
{
	for(int i=0; i<static_cast<int>(clientOrders.orders.size()); i++)
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
			
			UnitInput.push_back(tmp_order);
		}
		
		
		else if(order_type == -1) // A COMMAND message from GOD (server)
		{
			
			Order tmp_order;
			tmp_order.plr_id = -1;
			ss >> tmp_order.frameID;
			ss >> tmp_order.serverCommand;
			
			for(int i=0; i<UnitInput.size(); i++)
				cerr << "(" << UnitInput[i].frameID << ", " << UnitInput[i].plr_id << ")" << endl;
			
			cerr << "GOT A SERVER MESSAGE: " << tmp_order.serverCommand << " " << tmp_order.frameID << "(current frame: " << simulRules.currentFrame << ", allowed frame: " << simulRules.allowedFrame << ")" << endl;
			
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
				client_state = 1;
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
			else if(cmd == "UNIT") // unit copy message
			{
				cerr << "Creating a new unit as per instructions" << endl;
				int unitID;
				ss >> unitID;
				world.addUnit(unitID);
				ss >> world.units[unitID].angle >> world.units[unitID].keyState >> world.units[unitID].position.x.number >> world.units[unitID].position.y.number >> world.units[unitID].position.h.number >> world.units[unitID].velocity.x.number >> world.units[unitID].velocity.y.number >> world.units[unitID].velocity.h.number;
			}
			else if(cmd == "SIMUL")
			{
				cerr << "Set simulRules to instructed state" << endl;
				ss >> simulRules.currentFrame >> simulRules.windowSize >> simulRules.frameSkip >> simulRules.numPlayers >> simulRules.allowedFrame;
				cerr << simulRules.currentFrame << " " << simulRules.windowSize << " " << simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << endl;
			}
			else if(cmd == "CLIENT_STATE")
			{
				ss >> client_state;
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
			cerr << "Got a copy of an old message" << endl;
			Order tmp_order;
			ss >> tmp_order.frameID >> tmp_order.plr_id >> tmp_order.keyState >> tmp_order.mousex >> tmp_order.mousey >> tmp_order.serverCommand;
			UnitInput.push_back(tmp_order);
		}
		else
		{
			cerr << "HOLY FUCK! I DONT UNDERSTAND SHIT :G" << endl;
		}
	}
	
	clientOrders.orders.clear();
}









void Game::client_tick()
{

	// check if we have new msgs from the server.
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
	
	
	
	
	
	// if state_descriptor == 0, the userIO
	// is used by HOST functions. Do not interfere.
	if( ((state == "client") || (state_descriptor != 0)) && (client_state != 0))  
	{
		
		userio.checkEvents();
		
		// this is acceptable because the size is guaranteed to be insignificantly small
		sort(UnitInput.begin(), UnitInput.end());
		
		// handle any server commands intended for this frame
		while((UnitInput.back().plr_id == -1) && (UnitInput.back().frameID == simulRules.currentFrame))
		{
			Order server_command = UnitInput.back();
			UnitInput.pop_back();
			handleServerMessage(server_command);
		}
		
		if( (simulRules.currentFrame < simulRules.allowedFrame) && (fps_world.need_to_draw(SDL_GetTicks()) == 1) )
		{
			if( (UnitInput.back().plr_id == -1) && (UnitInput.back().frameID != simulRules.currentFrame) )
				cerr << "ERROR: ServerCommand for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;
			
			fps_world.insert();
			
			int keyState = userio.getKeyChange();
			int x, y;
			
			userio.getMouseChange(x, y);
			int frame = simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize;
			
			if(myID >= 0)
			{
				stringstream inputMsg;
				string msg;
				inputMsg << "1 " << myID << " " << frame << " " << keyState << " " << x << " " << y << "#";
				msg = inputMsg.str();
				clientSocket.write(msg);
			}
			
			// update commands of player controlled characters
			// for(int i=0; i<simulRules.numPlayers; i++)
			while(UnitInput.back().frameID == simulRules.currentFrame)
			{
				Order tmp = UnitInput.back();
				UnitInput.pop_back();
				
				if(tmp.plr_id == -1)
				{
					cerr << "MOTHERFUCKER FUCKING FUCK YOU MAN?= JUST FUCK YOU!!" << endl;
					break;
				}
				
				world.units[tmp.plr_id].updateInput(tmp.keyState, tmp.mousex, tmp.mousey);
			}
			
			view.updateInput(keyState, x, y);
			
			// run simulation for one WorldFrame
			world.worldTick();
			simulRules.currentFrame++;
		}
	}
	
}

