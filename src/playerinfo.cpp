
#include "playerinfo.h"

#include <sstream>

std::string PlayerInfo::getDescription()
{
	std::stringstream desc;
	desc << kills << " " << deaths << " " << name;
	return desc.str();
}

