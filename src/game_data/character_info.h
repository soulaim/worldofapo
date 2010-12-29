
#include "../playerinfo.h"
#include "../unit.h"

#include <sstream>

struct CharacterInfo
{
	PlayerInfo playerInfo;
	Unit unit;
	
	std::string getDescription()
	{
		if(playerInfo.name == "")
		{
			std::cerr << "WARNING: Name was empty on a playerInfo to be sent!" << std::endl;
			playerInfo.name = "nameless";
		}
		
		if(unit.name == "")
		{
			std::cerr << "WARNING: Name was empty on a unit to be sent!" << std::endl;
			unit.name = "nameless";
		}
		
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
		// std::cerr << "INTERPRETING CHARACTERINFOMSG\n" << str << std::endl;
		
		std::stringstream ss(str);
		playerInfo.readDescription(ss);
		
		// three segments of metadata to discard
		std::string dummy;
		ss >> dummy >> dummy >> dummy;
		unit.init();
		unit.handleCopyOrder(ss);
		
		// std::cerr << unit.name << ": " << unit.hitpoints << std::endl;
	}
};

