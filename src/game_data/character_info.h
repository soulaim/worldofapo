
#include "../playerinfo.h"
#include "../unit.h"

#include <sstream>

struct CharacterInfo
{
	PlayerInfo playerInfo;
	Unit unit;
	
	std::string getDescription()
	{
		std::string plr_msg  = playerInfo.getDescription();
		std::string unit_msg = unit.copyOrder(-1);
		
		std::string final_msg;
		final_msg.append(plr_msg);
		final_msg.append(" ");
		final_msg.append(unit_msg);
		return final_msg;
	}
	
	void readDescription(std::string& str)
	{
		std::stringstream ss(str);
		playerInfo.readDescription(ss);
		
		// three segments of metadata to discard
		std::string dummy;
		ss >> dummy >> dummy >> dummy;
		unit.init();
		unit.handleCopyOrder(ss);
		
		std::cerr << unit.name << ": " << unit.hitpoints << std::endl;
	}
};

