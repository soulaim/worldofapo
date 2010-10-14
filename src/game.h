
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
	
	World world;
	UserIO userio;
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
	
	std::map<int, PlayerInfo> Players;
	PlayerInfo localPlayer;
	
	int state_descriptor;
	int client_state;
	
	std::string clientCommand;
	
	StateInfo simulRules; // rules for running the simulation.
	int myID;
	
	void joinInternetGame(const std::string&);
	void endGame();
	
	void reset();
	void init();
	void readConfig();
	
	void handleServerMessage(const Order&);
	void processClientMsgs();
	void camera_handling();
	int connectMenu();

	void check_messages_from_server();
	void handleClientLocalInput();
	bool client_tick_local();
	void process_game_input();

	void enableGrab();
	void disableGrab();
	
public:
	Game(Graphics* view);
	std::string state;
	
	void menu_tick();
	void draw();
};


#endif

