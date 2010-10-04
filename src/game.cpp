
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

void Game::reset()
{
	cerr << "Reseting client state." << endl;
	
	serverAllow = 0;
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
	
	readConfig();
	
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
	if (e)
	  name.assign(e);
	else
	  name = "failname";
	localPlayer.name = name;
}

void Game::makeLocalGame()
{
	myID = -1;
	string host = "localhost";
	state = "host";
	
	int port = 12345;
	while(serverSocket.init_listener(port) == 0)
		port--;
	
	serverSocket.alive = true;
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
		host_tick();
	}
	
	client_tick();
	
	if((world.units.find(myID) != world.units.end()) && (myID >= 0))
	{
		world.viewTick();
		view.draw(world.models, world.lvl);
	}
	else
	{
		cerr << "looks like my unit doesnt, exist! Not really sure what to draw now :G" << endl;
	}
	
	if(state == "menu")
	{
		if(myID >= 0)
			reset();
		
		menuQuestions();
	}
}

