#ifndef DEDICATEDSERVER_H
#define DEDICATEDSERVER_H

#include "../world.h"
#include "../visualworld.h"
#include "../ordercontainer.h"
#include "../fps_manager.h"
#include "../order.h"
#include "../playerinfo.h"

#include "../net/socket.h"
#include "../net/socket_handler.h"

#include "../hasproperties.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <sstream>

#include "../game_data/character_info.h"

// information regarding how much of the simulation is allowed to play now,
// at which point of the simulation we are now,
// frame skips, window sizes..
struct StateInfo
{
	StateInfo(): windowSize(5), frameSkip(1), currentFrame(0), numPlayers(0)
	{
		reset();
	}
	
	void reset()
	{
		windowSize = 5;
		frameSkip = 1;
		currentFrame = 0;
		allowedFrame = 0;
		numPlayers = 0;
	}
	
	unsigned windowSize;
	unsigned frameSkip;
	unsigned currentFrame;
	unsigned allowedFrame;
	unsigned numPlayers;
};


class DedicatedServer : public HasProperties
{
	enum { SERVER_ID = -1 };

	FPS_Manager fps_world;
	VisualWorld visualworld;
	World world;
	
	SocketHandler sockets;  // children, other processes connected to my hosted game.
	
	OrderContainer clientOrders;
	
	std::vector<std::string> serverMsgs; // messages to be sent by the host
	std::vector<Order> UnitInput;
	
	std::map<std::string, CharacterInfo> dormantPlayers;
	std::map<int        , PlayerInfo> Players;
	
	enum PauseState
	{
		WAITING_PLAYERS,
		PAUSED,
		RUNNING
	};
	PauseState pause_state;
	
	// sign-in handling
	void playerStartingChoice(int, std::string);
	void handleSignInMessage(int, std::string);
	void disconnect(int leaver);
	
	// some message sending stuff
	void serverSendMonsterSpawn(int n, int t);
	void serverSendMonsterSpawn();
	void serverSendRequestPlayerNameMessage(int player_id);
	
	unsigned serverAllow;
	StateInfo simulRules; // rules for running the simulation.
	
	void init();
	
	void sendWorldCopy(const std::string& areaName, int plrID);

	void parseClientMsg(const std::string& msg, int player_id, PlayerInfo& player);
	void handleWorldEvents();
	void simulateWorldFrame();

	void ServerHandleServerMessage(const Order&);
	void check_messages_from_clients();
	void processClientMsgs();
	void processClientMsg(const std::string& msg);
	void processServerMsgs();
	
	void send_to_all(const std::string& msg);
	void acceptConnections();
public:
	DedicatedServer();
	
	bool start(int port);
	void host_tick();
	void draw();
};


#endif

