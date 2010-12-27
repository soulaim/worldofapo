
#include "dedicated.h"

using namespace std;

void DedicatedServer::sendWorldCopy(const string& areaName, int plr_ID)
{
	World& world = areas.find(areaName)->second;
	
	// send world generating parameters
	string world_parameters = world.generatorMessage();
	sockets.write(plr_ID, world_parameters);
	
	// send new player the current state of the world: units
	for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
	{
		string unitcopy = iter->second.copyOrder(iter->first);
		sockets.write(plr_ID, unitcopy);
	}
	
	// send new player the current state of the world: projectiles
	for(map<int, Projectile>::iterator iter = world.projectiles.begin(); iter != world.projectiles.end(); iter++)
	{
		sockets.write(plr_ID, iter->second.copyOrder(iter->first));
	}
	
	for(map<int, WorldItem>::iterator iter = world.items.begin(); iter != world.items.end(); ++iter)
	{
		sockets.write(plr_ID, iter->second.copyOrder(iter->first));
	}
	
	// send new player current pending orders
	for(size_t i = 0; i < UnitInput.size(); ++i)
	{
		sockets.write(plr_ID, UnitInput[i].copyOrder());
	}
}

void DedicatedServer::serverSendMonsterSpawn()
{
	stringstream tmp_msg;
	tmp_msg << "-1 " << (serverAllow+10) << " 10#";
	serverMsgs.push_back(tmp_msg.str());
}

void DedicatedServer::serverSendMonsterSpawn(int n, int team)
{
	for(int i=0; i<n; i++)
	{
		stringstream tmp_msg;
		tmp_msg << "-1 " << (serverAllow+10+i) << " 15 " << team << "#";
		serverMsgs.push_back(tmp_msg.str());
	}
}

void DedicatedServer::serverSendRequestPlayerNameMessage(int player_id)
{
	cerr << "Sending a request to the new player to identify himself!" << endl;
	string msg = "-2 GIVE_NAME#";
	sockets.write(player_id, msg);
}

