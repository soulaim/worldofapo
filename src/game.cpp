
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
	serverAllow = 0;
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
		view.draw(world.models, world.lvl);
	}
	else
	{
		cerr << "looks like my unit doesnt, exist! Not really sure what to draw now :G" << endl;
	}
	
	if(state == "menu")
		menuQuestions();
}

