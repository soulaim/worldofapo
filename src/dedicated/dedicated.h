
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
	
public:
	MU_Socket serverSocket; // for hosting games
private:
	MU_Socket clientSocket; // for connecting to all games (also local games)
	SocketHandler sockets;  // children, other processes connected to my hosted game.
	
	OrderContainer clientOrders;
	
	std::vector<std::string> serverMsgs; // messages to be sent by the host
	std::vector<Order> UnitInput;
	
	std::map<std::string, PlayerInfo> dormantPlayers;
	std::map<int        , PlayerInfo> Players;
	PlayerInfo localPlayer;
	
	int state_descriptor;
	int client_state;
	
	// sign-in handling
	void playerStartingChoice(int, std::string);
	void handleSignInMessage(int, std::string);
	
	// some message sending stuff
	void serverSendMonsterSpawn(int n);
	void serverSendMonsterSpawn();
	void serverSendRequestPlayerNameMessage(int player_id);
	
	unsigned serverAllow;
	StateInfo simulRules; // rules for running the simulation.
	int myID;
	
	void init();
	void readConfig();
	
	void handleWorldEvents();

	void sendWorldCopy(const string& areaName, int plrID);
	void update_kills();
	void update_deaths();

	// for dedicated server
	void ServerProcessClientMsgs();
	void ServerHandleServerMessage(const Order&);
	
	void send_to_all(const std::string& msg);
	void acceptConnections();
public:
	DedicatedServer();
	std::string state;
	
	bool start(int port);
	void host_tick();
	void draw();
};


#endif

