
#ifndef H_GAME_
#define H_GAME_

#include "../world.h"
#include "../ordercontainer.h"
#include "../fps_manager.h"
#include "../order.h"
#include "../playerinfo.h"

#include "../net/socket.h"
#include "../net/socket_handler.h"

#include <SDL/SDL.h>

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <sstream>


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


class DedicatedServer
{
	FPS_Manager fps_world;
	World world;
	
	SocketHandler sockets;  // children, other processes connected to my hosted game.
	
	OrderContainer clientOrders;
	
	std::vector<std::string> serverMsgs; // messages to be sent by the host
	std::vector<Order> UnitInput;
	
	std::map<std::string, PlayerInfo> dormantPlayers;
	std::map<int        , PlayerInfo> Players;
	
	enum PausedState
	{
		PAUSED = 0,
		GO
	};
	PausedState state_descriptor;
	PausedState client_state; // Could these separate pause states be merged?
	
	// sign-in handling
	void playerStartingChoice(int, std::string);
	void handleSignInMessage(int, std::string);
	void disconnect(int leaver);
	
	// some message sending stuff
	void serverSendMonsterSpawn(int n);
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
	void processClientMsgs();
	void processClientMsg(const std::string& msg);
	
	void send_to_all(const std::string& msg);
	void acceptConnections();
public:
	DedicatedServer();
	
	bool start(int port);
	void host_tick();
	void draw();
};


#endif

