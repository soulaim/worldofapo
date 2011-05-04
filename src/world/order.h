
#ifndef H_ORDER_
#define H_ORDER_

#include <string>
#include <sstream>

struct Order
{
	Order(): cmd_type(0), frameID(0), plr_id(0), keyState(0), mousex(0), mousey(0), serverCommand(0), mouseButtons(0) {}
	
	int cmd_type;
	
	unsigned frameID;
	int plr_id;
	
	int keyState;
	int mousex;
	int mousey;
	int serverCommand;
	int mouseButtons;
	
	bool operator < (const Order& a) const
	{
		if(frameID == a.frameID)
			return plr_id > a.plr_id;
		return frameID > a.frameID;
	}
	
	void handleCopyOrder(std::stringstream& ss)
	{
		ss >> plr_id >> frameID >> keyState >> mousex >> mousey >> serverCommand >> mouseButtons >> cmd_type;
	}
	
	std::string copyOrder() const
	{
		std::stringstream input_msg;
		input_msg << "-4 " << plr_id << " " << frameID << " " << keyState << " " << mousex << " " << mousey << " " << serverCommand << " " << mouseButtons << " " << cmd_type << "#";
		return input_msg.str();
	}
};

#endif

