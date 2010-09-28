
#include "game.h"

#include <iostream>
#include <sstream>

using namespace std;


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
      
      UnitInput.push(tmp_order);
    }
    
    
    else if(order_type == -1) // A message from GOD (server)
    {
      string cmd;
      ss >> cmd;
      
      if(cmd == "PLRID")
      {
	// I GOT MY PLAYER ID :) HAPPY!
	ss >> myID;
	cerr << "I got my player ID! :) It is now set to: " << myID << endl;
      }
      else if(cmd == "NUMPLAYERS")
      {
	// GAME STARTS NOW! :D (AND I GET TO KNOW HOW MANY PLAYERS ARE IN)
	ss >> simulRules.numPlayers;
	client_state = 1; // ready to rock :)
	
	fps_world.setStartTime( SDL_GetTicks() );
	for(int player = 0; player < simulRules.numPlayers; player++)
	    world.addUnit();
	
	simulRules.allowedFrame = simulRules.windowSize * simulRules.frameSkip;
	
	// WE MUST CREATE DUMMY INPUT FOR ALL PLAYERS FOR THE FIRST windowSize frames!
	for(int player = 0; player < simulRules.numPlayers; player++)
	{
	  for(int frame = 0; frame < simulRules.windowSize * simulRules.frameSkip; frame++)
	  {
	    Order dummy_order;
	    dummy_order.plr_id = player;
	    dummy_order.frameID = frame;
	    
	    UnitInput.push(dummy_order);
	  }
	}
	
	cerr << "Got number of players. The local value is now: " << simulRules.numPlayers << endl;
      }
      else
      {
	cerr << "Got a message from GOD, but don't really know what it means :(" << endl;
      }
      
      
    }
    else
    {
      cerr << "HOLY FUCK! I DONT UNDERSTAND SHIT :G" << endl;
    }
  
  }
  
  clientOrders.orders.clear();
}




Game::Game(): fps_world(0)
{
  client_state = 0;
  myID = -1;
  state_descriptor = 0;
  init();
}

void Game::init()
{
  
//  serverSocket.init_listener(12345);
//  clientSocket.conn_init("dcb-16", 12345);
//  sockets;
  myID = -1;
  state = "menu";
  view.loadObjects("data/parts.dat");
  view.megaFuck(); // blah..
  
  Image img;
  string grassFile = "data/grass.png";
  img.loadImage(grassFile);
  view.buildTexture(img);
  
}


void Game::makeLocalGame()
{
  myID = -1;
  string host = "127.0.0.1";
  serverSocket.init_listener(12345);
  clientSocket.conn_init(host, 12345);
}

void Game::joinInternetGame(string ip_addr)
{
  myID = -1;
  state = "client";
  clientSocket.conn_init(ip_addr, 12345);
}

void Game::menuQuestions()
{
  char val = userio.getSingleChar();
  char str[2];
  str[0] = val;
  str[1] = 0;
  
  // word ready, process the command :D
  if(val == '#')
  {
    stringstream ss(menuWord);
    string word1;
    ss >> word1;
    if(word1 == "connect")
    {
      string ip_addr;
      ss >> ip_addr;
      joinInternetGame(ip_addr);
    }
    
    if(menuWord == "start local")
    {
      cerr << "Hosting a new game.." << endl;
      makeLocalGame();
      state = "host";
    }
    
    // serves as a START button. Could also be used for UNPAUSE functionality?
    if( (menuWord == "go") && (state_descriptor == 0) )
    {
      state_descriptor = 1;
    }
    
    menuWord = "";
    return;
  }
  
  if(val != 0)
  {
    menuWord.append(str);
    cerr << "current menuword: " << menuWord << endl;
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
    // accept any incoming connections
    if(serverSocket.readyToRead() == 1)
    {
      cerr << "looks like someone is connecting :O" << endl;
      serverSocket.accept_connection(sockets);
      
      // tell the new player what his player ID is.
      stringstream playerID_msg;
      playerID_msg << "-1 PLRID " << (sockets.sockets.size() - 1) << "#";
      
      string msg = playerID_msg.str();
      sockets.sockets.back().write(msg);
    }
    
    // if there's any data to be read from clients, then read it
    sockets.get_readable();
    sockets.read_selected();
    
    // mirror any client commands to all clients
    for(int i=0; i<static_cast<int>(sockets.sockets.size()); i++)
    {
      for(int k=0; k<static_cast<int>(sockets.sockets[i].msgs.size()); k++)
      {
	sockets.sockets[i].msgs[k].append("#");
	for(int j=0; j<static_cast<int>(sockets.sockets.size()); j++)
	  sockets.sockets[j].write(sockets.sockets[i].msgs[k]);
      }
      sockets.sockets[i].msgs.clear();
    }
    
    
    if(state_descriptor == 0)
    {
      menuQuestions();
      
      if(state_descriptor == 1)
      {
	// THE GAME HAS STARTED! SEND THIS INFO TO ALL CONNECTED PLAYERS.
	// STOP LISTENING TO NEW CONNECTIONS?
	stringstream player_count;
	player_count << "-1 NUMPLAYERS " << sockets.sockets.size() << "#";
	serverMsgs.push_back(player_count.str());
      }
    }
    
    
    // transmit serverMsgs to players
    for(int k=0; k   < static_cast<int>(serverMsgs.size()); k++)
      for(int i=0; i < static_cast<int>(sockets.sockets.size()); i++)
	sockets.sockets[i].write(serverMsgs[k]);
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
	
//      cerr << "Client received msg: " << msg << endl;
      
      clientOrders.insert(msg); // give it to orderhandler to be parsed down to single commands
      processClientMsgs();
    }
    
    // update the state of local user input
    if( ((state == "client") || (state_descriptor != 0)) && (client_state != 0))  
      // if state_descriptor == 0, the userIO
      // is used by HOST functions. Do not interfere.
    {
      userio.checkEvents();
    
      // if allowed to simulate, send userIO to server for world frame i.
      // allowed to simulate if fps control agrees & server has given permission
      if(numInputs.find(simulRules.currentFrame+1) != numInputs.end())
      {
	if(numInputs[simulRules.currentFrame+1] == simulRules.numPlayers)
	{
//	  cerr << "Have all inputs. Incrementing allowed simulation range" << endl;
	  simulRules.allowedFrame++; // += simulRules.frameSkip;
	  numInputs.erase(simulRules.currentFrame+1);
	}
      }
	
      if( (simulRules.currentFrame < simulRules.allowedFrame) && (fps_world.need_to_draw(SDL_GetTicks()) == 1) )
      {
	fps_world.insert();
	int keyState = userio.getKeyChange();
	int x, y;
	userio.getMouseChange(x, y);
	int frame = simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize;
	
	stringstream inputMsg;
	string msg;
	inputMsg << "1 " << myID << " " << frame << " " << keyState << " " << x << " " << y << "#";
	msg = inputMsg.str();
	
	clientSocket.write(msg);
	
	// update commands of player controlled characters
	for(int i=0; i<simulRules.numPlayers; i++)
	{
	  Order tmp = UnitInput.top();
	  UnitInput.pop();
	  
//	  cerr << "HERO INPUT #" << simulRules.currentFrame << " " << tmp.frameID << " for hero " << tmp.plr_id << endl;
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
    
    if(static_cast<int>(world.units.size()) > myID)
    {
      view.setCamera(world.units[myID].position);
      view.draw(world.models, world.lvl);
    }
  }
  
  
  if(state == "menu")
    menuQuestions();
}

