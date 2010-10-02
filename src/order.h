
#ifndef H_ORDER_
#define H_ORDER_

struct Order
{
	Order(): frameID(0), plr_id(0), keyState(0), mousex(0), mousey(0), serverCommand(0) {}
	
	int frameID;
	int plr_id;
	
	int keyState;
	int mousex;
	int mousey;
	
	int serverCommand;
	
	bool operator < (const Order& a) const
	{
		if(frameID == a.frameID)
			return plr_id > a.plr_id;
		return frameID > a.frameID;
	}
};

#endif

