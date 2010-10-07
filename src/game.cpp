
#include "game.h"
#include "texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL/SDL_mixer.h>

using namespace std;



Mix_Music *music = NULL;
void musicDone();

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
	enableGrab(); // FIXME: find better place
}

void Game::init()
{
	reset();

	SDL_Init(SDL_INIT_AUDIO);
	Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, (short) MIX_DEFAULT_FORMAT, 2, 1024);

	music = Mix_LoadMUS("data/theme.ogg");
	Mix_PlayMusic(music, -1);
	Mix_HookMusicFinished(musicDone);
	
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

void musicDone()
{
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	Mix_CloseAudio();
	music = NULL;
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
	
	view.setTime( SDL_GetTicks() );
	if((world.units.find(myID) != world.units.end()) && (myID >= 0))
	{
		world.viewTick();
		view.tick();
		view.draw(world.models, world.lvl);
	}
	else
	{
		cerr << "looks like my unit doesnt exist! Not really sure what to draw now :G" << endl;
	}
	
	if(state == "menu")
	{
		if(myID >= 0)
			reset();
		
		menuQuestions();
	}
}

