

#include "dedicated/dedicated.h"

#include <iostream>

using std::cerr;
using std::endl;

void DedicatedServer::handle(const BulletHitEvent&)
{
	
}

void DedicatedServer::handle(const DevourEvent&)
{
	
}

void DedicatedServer::handle(const DeathPlayerEvent& event)
{
	if( (Players.find(event.actor_id) != Players.end()) )
	{
		Players[event.actor_id].kills++;
		cerr << Players[event.actor_id].name << " has killed" << endl;
	}
	
	if( (Players.find(event.target_id) != Players.end()) )
	{
		Players[event.target_id].deaths++;
		cerr << Players[event.target_id].name << " has died" << endl;
	}
}

void DedicatedServer::handle(const DeathNPCEvent& event)
{
	if( (Players.find(event.actor_id) != Players.end()) )
	{
		Players[event.actor_id].kills++;
		cerr << Players[event.actor_id].name << " has killed" << endl;
	}
}

void DedicatedServer::handle(const GameOver& event)
{
	cerr << "Game result: " << event.win << endl;
}

