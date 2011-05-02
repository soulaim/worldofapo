#ifndef H_GAME_
#define H_GAME_

#include "world/ordercontainer.h"
#include "misc/fps_manager.h"
#include "world/order.h"
#include "world/playerinfo.h"
#include "misc/keymanager.h"
#include "misc/hasproperties.h"

#include "net/socket_handler.h"
#include "game_data/character_info.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>


class World;

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


class Game : public HasProperties
{
	World* world;

	Network::SocketHandler clientSocket;

	OrderContainer clientOrders;
	std::vector<std::string> serverMsgs; // messages to be sent by the host
	std::vector<Order> UnitInput;
	
	std::map<int, CharacterInfo> SpawningHeroes;
	
public:
	enum { NO_ID = -2, SERVER_ID = -1 };
	
	std::map<int, PlayerInfo> Players; // TODO: make private?
	PlayerInfo localPlayer;
	
	std::vector<HasProperties> meta_events;
private:

	enum PausedState
	{
		PAUSED = 0,
		GO
	};
	
	PausedState paused_state;
	StateInfo simulRules; // rules for running the simulation.
	
	void handleServerMessage(const Order&);
	
	bool getHeroes(std::map<std::string, CharacterInfo>&);
	
public:
	Game(World* world);
	
	void init();
	void readConfig(); // TODO: maybe shouldn't be done here.
	
	int myID; // TODO: make private and getter?

	void set_current_frame_input(int keystate, int x, int y, int mousepress);
	void send_chat_message(const std::string&);
	void endGame();

	bool check_messages_from_server();
	void processClientMsgs();

	bool client_tick_local();
	void TICK();
	void reset();
	
	void write(int id, const std::string&);

	bool internetGameGetHeroes(const std::string&, std::map<std::string, CharacterInfo>&);
	void internetGameSelectHero(const std::string&);
	
	bool paused() const;
	void process_received_game_input(); // TODO: No private :((
};


#endif

