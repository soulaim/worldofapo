#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <string>

class PlayerInfo {

public:
	PlayerInfo(): kills(0), deaths(0) {}
	std::string name;
	int kills;
	int deaths;
};

#endif
