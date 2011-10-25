
#ifndef H_NETWORKMESSAGE
#define H_NETWORKMESSAGE

#include <string>

class NetworkMessage
{
public:

	enum ServerMessageType
	{
		PAUSE_MESSAGE_ID = 3,
		SPAWN_MONSTER_MESSAGE_ID = 10,
		SPAWN_MONSTER_TO_TEAM_MESSAGE_ID = 15,
		CHANGE_TEAM_MESSAGE_ID = 11,
		PLAYER_HAS_DISCONNECTED_MESSAGE_ID = 100,
		CREATE_BASE_BUILDINGS_MESSAGE_ID = 18,
		ADD_HERO_MESSAGE_ID = 1,
		SET_PLAYER_ID_MESSAGE_ID = 2,
		TOGGLE_GOD_MODE_MESSAGE_ID = 24,
		AREA_CHANGE_HERO_DESTROY_MESSAGE_ID = 7
	};

	enum InputMessageType
	{
		KEYSTATE_MESSAGE_ID   = 6,
		MOUSEPRESS_MESSAGE_ID = 7,
		MOUSEMOVE_MESSAGE_ID  = 8,
        STATINCREASE_MESSAGE_ID = 9
	};

	enum MessageType
	{
		COPY_ORDER_MESSAGE = -4,
		INSTANT_REACTION = -2,
		SERVER_ORDER = -1,
		PLAYER_INPUT = 1,
		PLAYERINFO_MESSAGE = 2,
		CHAT_MESSAGE = 3,
		ADMIN_ORDER_MESSAGE = 4
	};

	static std::string getKeyState(int myID, int frame, int keystate);
	static std::string getMousePress(int myID, int frame, int mousePress);
	static std::string getMouseMove(int myID, int frame, int x, int y);
    static std::string getStatIncrease(int myID, int frame, int);
};

#endif
