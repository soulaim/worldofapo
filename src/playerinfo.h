#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <string>

class PlayerInfo {

public:
	PlayerInfo(): kills(0), deaths(0), last_order(0), connectionState(0) {}
	
	const PlayerInfo& operator = (const PlayerInfo& a)
	{
		name = a.name;
		kills = a.kills;
		deaths = a.deaths;
		key = a.key;
		connectionState = a.connectionState;
		
		return *this;
	}
	
	std::string name;
	int kills;
	int deaths;
	
	unsigned last_order;
	int connectionState;
	std::string key;
	
	std::string getDescription();
};

#endif
