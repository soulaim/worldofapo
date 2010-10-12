


#include <SDL/SDL.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "../game.h"
#include "../logger.h"

using namespace std;

int main()
{
	cerr << "creating game object" << endl;
	Game master;
	
	master.state = "host";
	
	int port = 12345;
	while(master.serverSocket.init_listener(port) == 0)
		port--;
	
	master.serverSocket.alive = true;
	cerr << "Listening to port " << port << endl;
	
	
	while(true)
	{
		master.host_tick();
	}
	
	cerr << "apparently exiting now" << endl;
	return 0;
}
