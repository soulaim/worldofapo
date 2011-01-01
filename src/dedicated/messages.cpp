
#include "dedicated.h"

using namespace std;

void DedicatedServer::sendAreaParameters(const string& areaName, int plr_ID)
{
	World& world = areas.find(areaName)->second;
	
	// send world generating parameters
	string world_parameters = world.generatorMessage();
	sockets.write(plr_ID, world_parameters);
}

void DedicatedServer::sendWorldCopy(const string& areaName, int plr_ID)
{
	World& world = areas.find(areaName)->second;
	
	
	sockets.write(plr_ID, "-2 CLEAR_WORLD_PROPERTIES#");
	
	// send some aesthetical settings for the world
	{
		HasProperties& properties = area_settings[areaName];
		for(auto it = properties.intVals.begin(); it != properties.intVals.end(); it++)
		{
			stringstream settings_msg;
			settings_msg << "-2 CHANGE_PROPERTY " << it->first << " " << it->second << "#";
			sockets.write(plr_ID, settings_msg.str());
		}
		for(auto it = properties.strVals.begin(); it != properties.strVals.end(); it++)
		{
			stringstream settings_msg;
			settings_msg << "-2 CHANGE_PROPERTY " << it->first << " " << it->second << "#";
			sockets.write(plr_ID, settings_msg.str());
		}
	}
	
	{
		
		HasProperties& properties = world;
		for(auto it = properties.intVals.begin(); it != properties.intVals.end(); it++)
		{
			stringstream settings_msg;
			settings_msg << "-2 WORLD_PROPERTY I " << it->first << " " << it->second << "#";
			sockets.write(plr_ID, settings_msg.str());
		}
		for(auto it = properties.strVals.begin(); it != properties.strVals.end(); it++)
		{
			stringstream settings_msg;
			settings_msg << "-2 WORLD_PROPERTY S " << it->first << " " << it->second << "#";
			sockets.write(plr_ID, settings_msg.str());
		}
	}
	
	
	
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
	
	stringstream welcome_message;
	welcome_message << "3 -1 You have appeared in ^G" << areaName << "#";
	sockets.write(plr_ID, welcome_message.str());
	
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

