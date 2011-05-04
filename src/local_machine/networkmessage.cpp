
#include "local_machine/networkmessage.h"

void NetworkMessage::sendKeyState(Network::SocketHandler::Connection& conn, int myID, int keyState)
{
	conn << static_cast<int>(NetworkMessage::KEYSTATE_MESSAGE_ID) << " " << myID << " "  << 0 << " " << keyState << "#";
}

void NetworkMessage::sendMousePress(Network::SocketHandler::Connection& conn, int myID, int mousePress)
{
	conn << static_cast<int>(NetworkMessage::MOUSEPRESS_MESSAGE_ID) << " " << myID << " "  << 0 << " " << mousePress << "#";
}

void NetworkMessage::sendMouseMove(Network::SocketHandler::Connection& conn, int myID, int x, int y)
{
	conn << static_cast<int>(NetworkMessage::MOUSEMOVE_MESSAGE_ID) << " " << myID << " "  << 0 << " " << x << " " << y << "#";
}

