
#include "dedicated.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <sys/time.h>

using namespace std;

DedicatedServer::DedicatedServer(): fps_world(0)
{
	client_state = 0;
	myID = -1;
	state_descriptor = 0;
	serverAllow = 0;
	init();
}

void DedicatedServer::init()
{
	timeval t;
	gettimeofday(&t,NULL);
	long long milliseconds = t.tv_sec * 1000 + t.tv_usec / 1000;
	fps_world.reset(milliseconds);
	
	srand(time(0));
	readConfig();
}

void DedicatedServer::readConfig()
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
