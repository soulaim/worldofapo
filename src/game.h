
#ifndef H_GAME_
#define H_GAME_

#include "world.h"
#include "userio.h"
#include "graphics.h"
#include "ordercontainer.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include <string>
#include <vector>
#include <queue>
#include <map>

struct Order
{
  Order(): frameID(0), plr_id(0), keyState(0), mousex(0), mousey(0) {}
  
  int frameID;
  int plr_id;
  int keyState;
  int mousex;
  int mousey;
  
  bool operator < (const Order& a) const
  {
    if(frameID == a.frameID)
      return plr_id > a.plr_id;
    return frameID > a.frameID;
  }
};



// information regarding how much of the simulation is allowed to play now,
// at which point of the simulation we are now,
// frame skips, window sizes..
struct StateInfo
{
  StateInfo():windowSize(3), frameSkip(1), currentFrame(0), allowedFrame(0) {}
  
  void reset()
  {
    windowSize = 3;
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
  
  std::priority_queue<Order> UnitInput;
  std::map<int, int> numInputs;
  
  int state_descriptor;
  int client_state;
  std::string state;
  std::string menuWord;
  
  
  StateInfo simulRules;  // rules for running the simulation.
  int myID;
  
  void makeLocalGame();
  void joinInternetGame(std::string);
  void endGame();
  
  void init();
  void menuQuestions();
  
  
  void processClientMsgs();
  
public:
  Game();
  void start();
};


#endif

