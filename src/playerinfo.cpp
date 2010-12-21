
#include "playerinfo.h"

#include <sstream>

std::string PlayerInfo::getDescription()
{
	std::stringstream desc;
	desc << kills << " " << deaths << " " << name;
	return desc.str();
}

void PlayerInfo::readDescription(std::stringstream& input)
{
	input >> kills >> deaths >> name;
}

