
#include "game.h"
#include "texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>

// required for obtaining character keys from file. could be located in another file.
#include <fstream>

using namespace std;


Game::Game(): fps_world(0)
{
	client_state = 0;
	myID = -1;
	state_descriptor = 0;
	init();
	soundsystem.init();
}

void Game::reset()
{
	cerr << "Reseting client state." << endl;
	myID = -1;
	
	state = "menu";
	
	UnitInput.clear();
	world.terminate();
}

void Game::init()
{
	reset();
	
	view.loadObjects("data/parts.dat");
	view.megaFuck(); // blah..
	view.setPlayerInfo(&Players);
	
	readConfig();
	
	userio.init();
	
	// load some textures.
	// should not be done here. FIX
	TextureHandler::getSingleton().createTexture("grass", "data/grass.png");
	TextureHandler::getSingleton().createTexture("highground", "data/highground.png");
	TextureHandler::getSingleton().createTexture("mountain", "data/hill.png");
}

void Game::readConfig()
{
	//ifstream configFile("config.cfg");
	//configFile >> localPlayer.name;
	string name;
	char *e = getenv("USERNAME");
	if(e)
	  name.assign(e);
	else
	  name = "failname";
	localPlayer.name = name;
}

void Game::joinInternetGame(const string& hostName)
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
	
	ifstream keyCodes("myKeys");
	while(keyCodes.good() && !keyCodes.eof())
	{
		string key;
		keyCodes >> key;
		
		if(key.length() == 15)
		{
			stringstream ss;
			ss << "OPTION " << key << "#";
			cerr << "sending query for key " << key << endl;
			clientSocket.write(ss.str());
			
			cerr << "waiting for answer.." << endl;
			
			bool not_finished = true;
			while(not_finished)
			{
				if(clientSocket.readyToRead())
				{
					string msg = clientSocket.read();
					
					if(msg.size() == 0)
					{
						clientSocket.closeConnection();
						cerr << "Client connection has died during sign-in process. :(" << endl;
						state = "menu";
						return;
					}
					
					clientOrders.insert(msg); // give it to orderhandler to be parsed down to single commands
					
					for(size_t k=0; k<clientOrders.orders.size(); k++)
					{
						not_finished = false;
						string cmd;
						stringstream ss(clientOrders.orders[k]);
						ss >> cmd;
						
						if(cmd == "NO")
						{
							// my character was NOT found in the system :G
							cerr << "my character was not found :((" << endl;
						}
						else if(cmd == "YES")
						{
							
							getline(ss, cmd);
							cerr << "Want this one? (" << cmd << ") type \"OK\" and press enter to accept. Just enter to ignore" << endl;
							cin >> cmd;
							
							if(cmd == "OK")
							{
								cmd = "START ";
								cmd.append(key);
								cmd.append("#");
								clientSocket.write(cmd);
								
								cerr << "just sent the start command. breaking from this shit." << endl;
								return;
							}
						}
					}
				}
			}
		}
		else
		{
			cerr << "key of length " << key.length() << " found. The current standard is 15!" << endl;
		}
	}
	
	// start with a new character
	clientSocket.write("START NEW#");
	cerr << "Starting with a new character." << endl;
}


void Game::endGame()
{
	// disconnect from server
	
	// if I am the server, send to all clients termination message and shut down sockets.
	// shut down server socket.
	
	// reset client/server values to false
	
	// release local game resources.
}


void Game::draw()
{
	view.setTime( SDL_GetTicks() );
	if((world.units.find(myID) != world.units.end()) && (myID >= 0))
	{
		world.viewTick();
		view.tick();
		view.draw(world.models, world.lvl, world.units);
	}
}

