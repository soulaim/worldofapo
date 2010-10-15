#ifndef H_GAME_
#define H_GAME_

#include "world.h"
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
	FPS_Manager fps_world;
	
	World* world;

	MU_Socket clientSocket;

	OrderContainer clientOrders;
	std::vector<std::string> serverMsgs; // messages to be sent by the host
	
	std::vector<Order> UnitInput;
	
public:
	std::map<int, PlayerInfo> Players; // TODO: make private?
	PlayerInfo localPlayer;
private:

	enum PausedState
	{
		PAUSED = 0,
		GO
	};
	PausedState paused_state;
	StateInfo simulRules; // rules for running the simulation.

	void readConfig(); // TODO: maybe shouldn't be done here.
	
	void handleServerMessage(const Order&);
	void processClientMsgs();


public:
	Game(World* world);
	void init();

	int myID; // TODO: make private and getter?

	void set_current_frame_input(int keystate, int x, int y, int mousepress);
	void send_chat_message(const std::string&);
	void endGame();

	bool check_messages_from_server();
	bool client_tick_local();
	void TICK();
	void reset();

	bool joinInternetGame(const std::string&);

	bool paused() const;
	void process_received_game_input(); // TODO: No private :((
};


#endif

