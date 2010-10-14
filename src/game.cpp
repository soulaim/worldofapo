
#include "game.h"
#include "texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL/SDL_mixer.h>

using namespace std;

Game::Game(Graphics* v):
	fps_world(0),
	view(v)
{
	client_state = 0;
	myID = -1;
	state_descriptor = 0;
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
	
	view->loadObjects("data/parts.dat");
	view->megaFuck(); // blah..
	view->setPlayerInfo(&Players);
	
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
    enableGrab();

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
	
	cerr << "I think I got a connection, port: " << port << endl;
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
}

