
#ifndef H_GAME_
#define H_GAME_

#include "world.h"
#include "userio.h"
#include "graphics.h"
#include "ordercontainer.h"
#include "fps_manager.h"
#include "order.h"
#include "playerinfo.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include <string>
#include <vector>
#include <queue>
#include <map>

// information regarding how much of the simulation is allowed to play now,
// at which point of the simulation we are now,
// frame skips, window sizes..
struct StateInfo
{
	StateInfo():windowSize(5), frameSkip(1), currentFrame(0), numPlayers(0)
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
	
	int windowSize;
	int frameSkip;
	int currentFrame;
	int allowedFrame;
	int numPlayers;
};


class Game
{
	FPS_Manager fps_world;
	
	World world;
	UserIO userio;
	Graphics view;
	
	MU_Socket serverSocket; // for hosting games
	MU_Socket clientSocket; // for connecting to all games (also local games)
	SocketHandler sockets;  // children, other processes connected to my hosted game.
	
	OrderContainer clientOrders;
	OrderContainer serverOrders;
	
	std::vector<std::string> clientMsgs; // messages to be sent by the client
	std::vector<std::string> serverMsgs; // messages to be sent by the host
	
	std::vector<Order> UnitInput;
	
	map<int, PlayerInfo> Players;
	PlayerInfo localPlayer;
	
	int state_descriptor;
	int client_state;
	std::string state;
	
	std::string menuWord;
	std::string clientCommand;
	
	int serverAllow;
	StateInfo simulRules; // rules for running the simulation.
	int myID;
	
	void makeLocalGame();
	void joinInternetGame(std::string);
	void endGame();
	
	void reset();
	void init();
	void readConfig();
	
	void menuQuestions();
	
	void acceptConnections();
	void host_tick();
	
	void handleServerMessage(const Order&);
	void processClientMsgs();
	void client_tick();
	
public:
	Game();
	void start();
};


#endif

