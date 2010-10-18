

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "dedicated.h"
#include "../logger.h"

using namespace std;

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
		if(--port < 12000)
		{
			cerr << "STUPID USER :((" << endl;
			return 0;
		}
	}
	
	while(true)
	{
		master.host_tick();
	}
	
	cerr << "apparently exiting now" << endl;
	return 1;
}
