#ifndef DEDICATEDSERVER_H
#define DEDICATEDSERVER_H

#include "world/world.h"
#include "world/ordercontainer.h"
#include "world/order.h"
#include "world/playerinfo.h"

#include "graphics/visualworld.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include "misc/hasproperties.h"
#include "misc/idgenerator.h"
#include "misc/fps_manager.h"
#include "misc/messaging_system.h"

#include "local_machine/networkmessage.h"

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


class DedicatedServer : public HasProperties, public MessagingSystem<BulletHitEvent>, public MessagingSystem<DevourEvent>, public MessagingSystem<DeathPlayerEvent>, public MessagingSystem<DeathNPCEvent>, public MessagingSystem<GameOver>
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
	
	
	std::map<std::string, std::vector< std::vector<World::CheckSumType> > > checkSums;
	const unsigned checkSumVectorSize;
	
	
	FPS_Manager fps_world;
	VisualWorld visualworld; // one common visualworld
	
	std::map<std::string, World> areas;
	std::map<std::string, HasProperties> area_settings;
	
	Network::SocketHandler sockets;  // children, other processes connected to my hosted game.
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
	
	// in-system message delivery. has nothing to do with network connections.
	void deliverMessages();
	
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
	
	void simulateWorldFrame();

	void ServerHandleServerMessage(const Order&);
	void check_messages_from_clients();
	void processClientMsgs();
	void processClientMsg(const std::string& msg);
	void processServerMsgs();
	bool checkSumOK(int plrID, int frameID, std::vector<World::CheckSumType>& plr_cs);
	
	void send_to_all(const std::string& msg);
	void acceptConnections();

	void possible_sleep(int milliseconds) const;

public:
	DedicatedServer();
	
	void handle(const BulletHitEvent& event);
	void handle(const DevourEvent& event);
	
	void handle(const DeathPlayerEvent& event);
	void handle(const DeathNPCEvent& event);
	void handle(const GameOver& event);
	
	
	bool start(int port);
	void host_tick();
	void draw();
};


inline void socketWrite(Network::SocketHandler& sh, int id, const std::string& msg)
{
	sh.getConnection(id) << msg;
}


#endif

