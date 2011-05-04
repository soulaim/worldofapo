
#ifndef H_NETWORKMESSAGE
#define H_NETWORKMESSAGE

#include "net/socket_handler.h"

class NetworkMessage
{
public:
	enum
	{
		KEYSTATE_MESSAGE_ID   = 6,
		MOUSEPRESS_MESSAGE_ID = 7,
		MOUSEMOVE_MESSAGE_ID  = 8
	};
	
	static void sendKeyState(Network::SocketHandler::Connection& conn, int myID, int keystate);
	static void sendMousePress(Network::SocketHandler::Connection& conn, int myID, int mousePress);
	static void sendMouseMove(Network::SocketHandler::Connection& conn, int myID, int x, int y);
};

#endif
