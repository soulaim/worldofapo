#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <csignal>

#include "dedicated.h"
#include "../logger.h"

using namespace std;

bool running = true;

void stop_server(int dummy)
{
	dummy = dummy; // ...

	running = false;
}

int main()
{
	cerr << "starting logger" << endl;
	Logger log;
	log.open("server.log");
	
	cerr << "creating game object" << endl;
	DedicatedServer master;
	
	int port = 12345;
	while(!master.start(port))
	{
		if(--port < 12320)
		{
			cerr << "STUPID USER :((" << endl;
			return 0;
		}
	}

	signal(SIGINT, stop_server);
	cerr << "entering main loop" << endl;
	
	while(running)
	{
		master.host_tick();
	}
	
	// TODO: stop cleanly by closing connections and/or telling clients.

	cerr << "Server stopped" << endl;
	return 0;
}

