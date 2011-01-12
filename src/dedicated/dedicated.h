#ifndef DEDICATEDSERVER_H
#define DEDICATEDSERVER_H

#include "../world.h"
#include "graphics/visualworld.h"
#include "../ordercontainer.h"
#include "../fps_manager.h"
#include "../order.h"
#include "../playerinfo.h"

#include "../net/socket.h"
#include "../net/socket_handler.h"

#include "../hasproperties.h"
#include "../idgenerator.h"

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
	
	enum ConnectionState
	{
		SIGN_IN = 0,
		GAMEPLAY = 1,
		WAIT_WORLD_GEN = 2,
		ADMIN = 3,
		OBSERVER = 4
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
	
	
	std::map<std::string, std::vector< std::vector<World::CheckSumType> > > checkSums;
	const unsigned checkSumVectorSize;
	
	
	FPS_Manager fps_world;
	VisualWorld visualworld; // one common visualworld
	
	std::map<std::string, World> areas;
	std::map<std::string, HasProperties> area_settings;
	
	SocketHandler sockets;  // children, other processes connected to my hosted game.
	IDGenerator playerIDGenerator;
	
	OrderContainer clientOrders;
	
	std::map<int, CharacterInfo> SpawningHeroes;
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
	
	// player id handling
	int nextPlayerID();
	
	// sign-in handling
	void playerStartingChoice(int, std::string);
	void handleSignInMessage(int, std::string);
	bool startPlayerAreaChange(const std::string& next_area, int player_id);
	void disconnect(int leaver);
	
	// some message sending stuff
	void serverSendMonsterSpawn(int n, int t);
	void serverSendMonsterSpawn();
	void serverSendRequestPlayerNameMessage(int player_id);
	
	unsigned serverAllow;
	StateInfo simulRules; // rules for running the simulation.
	
	bool server_no_wait;
	
	void init();
	
	void changeArea(int playerID_val);
	void sendAreaParameters(const std::string& areaName, int plr_ID);
	void sendWorldCopy(const std::string& areaName, int plrID);
	void sendWorldContent(int playerID_val);

	void parseAdminMsg(const std::string& msg, int admin_id, PlayerInfo& admin);
	void parseClientMsg(const std::string& msg, int player_id, PlayerInfo& player);
	
	void handleWorldEvents();
	void simulateWorldFrame();

	void ServerHandleServerMessage(const Order&);
	void check_messages_from_clients();
	void processClientMsgs();
	void processClientMsg(const std::string& msg);
	void processServerMsgs();
	bool checkSumOK(int plrID, int frameID, std::vector<World::CheckSumType>& plr_cs);
	
	void send_to_all(const std::string& msg);
	void acceptConnections();

public:
	DedicatedServer();
	
	bool start(int port);
	void host_tick();
	void draw();
};


#endif

