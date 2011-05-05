
#ifndef H_NETWORKMESSAGE
#define H_NETWORKMESSAGE

#include <string>

class NetworkMessage
{
public:
	enum
	{
		KEYSTATE_MESSAGE_ID   = 6,
		MOUSEPRESS_MESSAGE_ID = 7,
		MOUSEMOVE_MESSAGE_ID  = 8
	};
	
	static std::string getKeyState(int myID, int frame, int keystate);
	static std::string getMousePress(int myID, int frame, int mousePress);
	static std::string getMouseMove(int myID, int frame, int x, int y);
};

#endif
