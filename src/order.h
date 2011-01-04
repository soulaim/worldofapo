
#ifndef H_ORDER_
#define H_ORDER_

#include <string>
#include <sstream>

struct Order
{
	Order(): frameID(0), plr_id(0), keyState(0), mousex(0), mousey(0), serverCommand(0), mouseButtons(0) {}
	
	unsigned frameID;
	int plr_id;
	
	int keyState;
	int mousex;
	int mousey;
	int serverCommand;
	int mouseButtons;
	unsigned long long checksum;
	
	bool operator < (const Order& a) const
	{
		if(frameID == a.frameID)
			return plr_id > a.plr_id;
		return frameID > a.frameID;
	}
	
	void handleCopyOrder(std::stringstream& ss)
	{
		ss >> frameID >> plr_id >> keyState >> mousex >> mousey >> serverCommand >> mouseButtons >> checksum;
	}
	
	std::string copyOrder() const
	{
		std::stringstream input_msg;
		input_msg << "-4 " << frameID << " " << plr_id << " " << keyState << " " << mousex << " " << mousey << " " << serverCommand << " " << mouseButtons << " " << checksum << "#";
		return input_msg.str();
	}
};

#endif

