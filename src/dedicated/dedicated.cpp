
#include "dedicated.h"

#include <iostream>
#include <sstream>
#include <algorithm>

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
