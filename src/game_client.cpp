
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

    cerr << "Creating dummy input for new hero." << endl;
    // WE MUST CREATE DUMMY INPUT FOR ALL PLAYERS FOR THE FIRST windowSize frames!
    for(int frame = 0; frame < simulRules.windowSize * simulRules.frameSkip; frame++)
    {
      Order dummy_order;
      dummy_order.plr_id = simulRules.numPlayers-1;
      cerr << "dummy order plrid: " << dummy_order.plr_id << endl;
      
      dummy_order.frameID = frame + simulRules.currentFrame;
      numInputs[dummy_order.frameID]++;
      
      UnitInput.push_back(dummy_order);
    }
    
    sort(UnitInput.begin(), UnitInput.end());
  }
  else if(server_msg.serverCommand == 2) // "set playerID" message
  {
    myID = server_msg.keyState; // trololol. nice place to store the info.
    cerr << "Setting local playerID at frame " << simulRules.currentFrame << " to value " << myID << endl;
    if(world.units.size() > myID)
    {
		cerr << "Binding camera to player " << myID << "\n";
        view.bindCamera(&world.units[myID]);
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
      
      if(numInputs.find( tmp_order.frameID ) == numInputs.end())
	numInputs[tmp_order.frameID] = 1;
      else
	numInputs[tmp_order.frameID]++;
      
      UnitInput.push_back(tmp_order);
    }
    
    
    else if(order_type == -1) // A COMMAND message from GOD (server)
    {
      
      Order tmp_order;
      tmp_order.plr_id = -1;
      ss >> tmp_order.frameID;
      ss >> tmp_order.serverCommand;
     
      cerr << "GOT A SERVER MESSAGE: " << tmp_order.serverCommand << " " << tmp_order.frameID << "(current frame: " << simulRules.currentFrame << ", allowed frame: " << simulRules.allowedFrame << ")" << endl;
      cerr << "numInputs[" << tmp_order.frameID << "] = " << numInputs[tmp_order.frameID] << endl;
      
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
      else if(cmd == "UNIT")
      {
	/*
	* THIS IS BROKEN. UNIT ID's ARE NOT TRANSMITTED
	* UNIT VELOCITIES ARE NOT TRANSMITTED. FIX.
	*/
	
	cerr << "Creating a new unit as per instructions" << endl;
	int unitID = world.nextUnitID(); // not safe :G
	world.addUnit(unitID);
	ss >> world.units[unitID].angle >> world.units[unitID].keyState >> world.units[unitID].position.x.number >> world.units[unitID].position.y.number >> world.units[unitID].position.h.number;
	
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
      
      if(numInputs.find(tmp_order.frameID) == numInputs.end())
	numInputs[tmp_order.frameID] = 1;
      else
        numInputs[tmp_order.frameID]++;
    }
    
    else
    {
      cerr << "HOLY FUCK! I DONT UNDERSTAND SHIT :G" << endl;
    }
    
  
  }
  
  clientOrders.orders.clear();
}
