
#include "dedicated.h"

using namespace std;

void DedicatedServer::sendWorldCopy(string areaName, int plr_ID)
{
	areaName = "";
	
	MU_Socket& connectingPlayer = sockets.sockets[plr_ID];
	
	// send new player the current state of the world: units
	for(map<int, Unit>::iterator iter = world.units.begin(); iter != world.units.end(); iter++)
		connectingPlayer.write(iter->second.copyOrder(iter->first));
	
	// send new player the current state of the world: projectiles
	for(map<int, Projectile>::iterator iter = world.projectiles.begin(); iter != world.projectiles.end(); iter++)
		connectingPlayer.write(iter->second.copyOrder(iter->first));
	
	// send new player current pending orders
	for(size_t i = 0; i < UnitInput.size(); ++i)
		connectingPlayer.write(UnitInput[i].copyOrder());
}

void DedicatedServer::serverSendMonsterSpawn()
{
	stringstream tmp_msg;
	tmp_msg << "-1 " << (serverAllow+10) << " 10#";
	serverMsgs.push_back(tmp_msg.str());
}

void DedicatedServer::serverSendMonsterSpawn(int n)
{
	for(int i=0; i<n; i++)
	{
		stringstream tmp_msg;
		tmp_msg << "-1 " << (serverAllow+10+i) << " 10#";
		serverMsgs.push_back(tmp_msg.str());
	}
}

void DedicatedServer::serverSendRequestPlayerNameMessage(int player_id)
{
	cerr << "Sending a request to the new player to identify himself!" << endl;
	sockets.sockets[player_id].write("-2 GIVE_NAME#");
}
