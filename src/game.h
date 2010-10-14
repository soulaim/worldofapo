
#ifndef H_GAME_
#define H_GAME_

#include "world.h"
#include "userio.h"
#include "graphics.h"
#include "ordercontainer.h"
#include "fps_manager.h"
#include "order.h"
#include "playerinfo.h"
#include "gamesound.h"

#include "net/socket.h"
#include "net/socket_handler.h"

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


class Game
{
	friend class Localplayer;

	FPS_Manager fps_world;
	
	World* world;
	Graphics* view;
	
public:
	MU_Socket serverSocket; // for hosting games
private:
	MU_Socket clientSocket; // for connecting to all games (also local games)
	SocketHandler sockets;  // children, other processes connected to my hosted game.
	
	OrderContainer clientOrders;
	
	std::vector<std::string> clientMsgs; // messages to be sent by the client
	std::vector<std::string> serverMsgs; // messages to be sent by the host
	
	std::vector<Order> UnitInput;
	
public:
	std::map<int, PlayerInfo> Players; // TODO: make private?
	PlayerInfo localPlayer;
private:
	
	int state_descriptor;

	enum PausedState
	{
		PAUSED = 0,
		GO
	};
	PausedState paused_state;
	
	StateInfo simulRules; // rules for running the simulation.

public:
	void joinInternetGame(const std::string&);
	
	void reset();
	void init();

private:
	void readConfig();
	
	void handleServerMessage(const Order&);
	void processClientMsgs();

	void check_messages_from_server();
	bool client_tick_local();
	void process_received_game_input();

	std::string state;

public:
	Game(Graphics* view, World* world);

	void set_current_frame_input(int keystate, int x, int y, int mousepress);
	int myID; // TODO: make getter?
	void send_chat_message(const std::string&);
	void endGame();
};


#endif

