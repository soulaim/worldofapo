
#include "local_machine/networkmessage.h"
#include <sstream>

std::string NetworkMessage::getKeyState(int myID, int frame, int keyState)
{
	std::stringstream ss;
	ss << static_cast<int>(NetworkMessage::KEYSTATE_MESSAGE_ID) << " " << myID << " "  << frame << " " << keyState << "#";
	return ss.str();
}

std::string NetworkMessage::getMousePress(int myID, int frame, int mousePress)
{
	std::stringstream ss;
	ss << static_cast<int>(NetworkMessage::MOUSEPRESS_MESSAGE_ID) << " " << myID << " "  << frame << " " << mousePress << "#";
	return ss.str();
}

std::string NetworkMessage::getStatIncrease(int myID, int frame, int stat) {
	std::stringstream ss;
	ss << static_cast<int>(NetworkMessage::STATINCREASE_MESSAGE_ID) << " " << myID << " "  << frame << " " << stat << "#";
	return ss.str();
}

std::string NetworkMessage::getMouseMove(int myID, int frame, int x, int y)
{
	std::stringstream ss;
	ss << static_cast<int>(NetworkMessage::MOUSEMOVE_MESSAGE_ID) << " " << myID << " "  << frame << " " << x << " " << y << "#";
	return ss.str();
}

