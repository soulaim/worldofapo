
#include "game.h"
#include "texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL/SDL_mixer.h>

using namespace std;

Game::Game(Graphics* v, World* w):
	fps_world(0),
	world(w),
	view(v)
{
	paused_state = PAUSED;
	myID = -1;
	state_descriptor = 0;
}

void Game::reset()
{
	cerr << "Reseting client state." << endl;
	myID = -1;
	
	state = "menu";
	
	UnitInput.clear();
	world->terminate();
}

void Game::init()
{
	reset();
	
	view->loadObjects("data/parts.dat");
	view->megaFuck(); // blah..
	view->setPlayerInfo(&Players);
	
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
	
	cerr << "I think I got a connection, port: " << port << endl;
}


void Game::endGame()
{
	clientSocket.closeConnection();
	
	// TODO: release local game resources?
}

void Game::set_current_frame_input(int keystate, int x, int y, int mousepress)
{
	int frame = simulRules.currentFrame + simulRules.frameSkip * simulRules.windowSize;
	
	if(myID >= 0)
	{
		stringstream inputMsg;
		string msg;
		inputMsg << "1 " << myID << " " << frame << " " << keystate << " " << x << " " << y << " " << mousepress << "#";
		msg = inputMsg.str();
		clientSocket.write(msg);
	}
}

void Game::send_chat_message(const std::string& clientCommand)
{
	stringstream tmp_msg;
	tmp_msg << "3 " << myID << " " << clientCommand << "#";
	clientSocket.write(tmp_msg.str());
}

