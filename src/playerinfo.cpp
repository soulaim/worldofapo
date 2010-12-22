
#include "playerinfo.h"

#include <sstream>
#include <iostream>

std::string PlayerInfo::getDescription()
{
	std::stringstream desc;
	
	if(key == "")
	{
		std::cerr << "ERROR: PlayerInfo::getDescription() FAIL : key = \"\"" << std::endl;
	}
	
	desc << " " << connectionState << " " << key << " " << kills << " " << deaths << " " << name;
	return desc.str();
}

void PlayerInfo::readDescription(std::stringstream& input)
{
	input >> connectionState >> key >> kills >> deaths >> name;
}

