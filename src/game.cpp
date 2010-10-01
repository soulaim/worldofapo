
#include "game.h"
#include "texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;



Game::Game(): fps_world(0)
{
  client_state = 0;
  myID = -1;
  state_descriptor = 0;
  init();
}

void Game::init()
{
  _playerID_next_player = 0;
  myID = -1;
  state = "menu";
  view.loadObjects("data/parts.dat");
  view.megaFuck(); // blah..
 
  // load some textures.
  // should not be done here.
  Image img;
  string grassFile = "data/grass.png";
  img.loadImage(grassFile);
  TextureHandler::getSingleton().createTexture("grass" ,img);

  string highGround = "data/highground.png";
  img.loadImage(highGround);
  TextureHandler::getSingleton().createTexture("highground" ,img);
  
  string mountain = "data/hill.png";
  img.loadImage(mountain);
  TextureHandler::getSingleton().createTexture("mountain" ,img);

}


void Game::makeLocalGame()
{
  myID = -1;
  string host = "127.0.0.1";
  
  int port = 12345;
  while(serverSocket.init_listener(port) == 0)
    port--;
  cerr << "Listening to port " << port << endl;
  
  clientSocket.conn_init(host, port);
}

void Game::joinInternetGame(string hostName)
{
  myID = -1;
  state = "client";
  int port = 12345;
  
  while(clientSocket.conn_init(hostName, port) == 0)
  {
    port--;
    if(port < 12000)
    {
      cerr << "superfail :(" << endl;
      exit(0);
    }
  }
}


void Game::endGame()
{
  // disconnect from server
  
  // if I am the server, send to all clients termination message and shut down sockets.
  // shut down server socket.

  // reset client/server values to false
  
  // release local game resources.
}


void Game::start()
{
  // THIS IS WHAT THE HOST DOES
  if(state == "host")
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
	}
	
      }
      i->second.msgs.clear();
    }
    
    if(leaver != -1) // there is a leaver!!
    {
      stringstream discCommand;
      discCommand << -1 << " " << (sockets.sockets[leaver].last_order + 1) << " 100 " << leaver << "#";
      serverMsgs.push_back( discCommand.str() );
      sockets.erase_id(leaver);
    }
    
    
    
    if(state_descriptor == 0)
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
  
  
  
  
  // THIS IS WHAT CLIENTS DO
  if((state == "client") || (state == "host"))
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
    
      if(numInputs.find(simulRules.currentFrame+1) != numInputs.end())
      {
	if(numInputs[simulRules.currentFrame+1] == simulRules.numPlayers)
	{
	  simulRules.allowedFrame++; // += simulRules.frameSkip;
	  numInputs.erase(simulRules.currentFrame+1);
	}
      }
      else
      {
	// if there are no players, then all commands have been received.
	if((simulRules.numPlayers == 0) && (simulRules.currentFrame == simulRules.allowedFrame))
	  simulRules.allowedFrame++;
      }
      
      
      
      
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
	
	/*
	for(int k=0; k<UnitInput.size(); k++)
	  cerr << UnitInput[k].frameID <<  " ";
	cerr << endl;
	*/
	
	
	// update commands of player controlled characters
	for(int i=0; i<simulRules.numPlayers; i++)
	{
	  Order tmp = UnitInput.back();
	  if(tmp.plr_id == -1)
	    break;
	  
	  UnitInput.pop_back();
	  
	  if(tmp.frameID != simulRules.currentFrame)
	    cerr << "Order for frame " << UnitInput.back().frameID << " encountered at frame " << simulRules.currentFrame << endl;

	  world.units[tmp.plr_id].updateInput(tmp.keyState, tmp.mousex, tmp.mousey);
	}
	
	
	
	// run simulation for one WorldFrame
	world.tick();
	simulRules.currentFrame++;
      }
      else
      {
//	cerr << "current frame: " << simulRules.currentFrame << "  allowedFrame: " << simulRules.allowedFrame << " mypos: " << world.units[0].position.x.number << " " << world.units[0].position.y.number << endl;
      }
    }
    
    if((static_cast<int>(world.units.size()) > myID) && (myID >= 0))
    {
      view.setCamera(world.units[myID].position);
      view.draw(world.models, world.lvl);
    }
  }
  
  
  if(state == "menu")
    menuQuestions();
}

