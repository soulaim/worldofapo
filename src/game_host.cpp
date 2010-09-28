
#include "game.h"

using namespace std;

void Game::acceptConnections()
{
  // accept any incoming connections
  if(serverSocket.readyToRead() == 1)
  {
    cerr << "looks like someone is connecting :O" << endl;
    serverSocket.accept_connection(sockets);
    
    // if game in progress, inform everyone else of a new connecting player TODO
  
  
    
    // send new player the current simulRules state
    stringstream simulRules_msg;
    simulRules_msg << "-2 SIMUL " << simulRules.currentFrame << " " << simulRules.windowSize << " " <<  simulRules.frameSkip << " " << simulRules.numPlayers << " " << simulRules.allowedFrame << "#";
    sockets.sockets.back().write(simulRules_msg.str());
    
    // send new player the current state of the world:
    for(int i=0; i<world.units.size(); i++)
    {
      stringstream hero_msg;
      hero_msg << "-2 UNIT " << world.units[i].angle << " " << world.units[i].keyState << " " << world.units[i].position.x.number << " " << world.units[i].position.y.number << " " << world.units[i].position.h.number << "#";
      sockets.sockets.back().write(hero_msg.str());
    }
    
    // send new player current pending orders
    for(int i=0; i<UnitInput.size(); i++)
    {
      stringstream input_msg;
      input_msg << "-4 " << UnitInput[i].frameID << " " << UnitInput[i].plr_id << " " << UnitInput[i].keyState << " " << UnitInput[i].mousex << " " << UnitInput[i].mousey << " " << UnitInput[i].serverCommand << "#";
      sockets.sockets.back().write(input_msg.str());
    }
    
    // tell the new player what his player ID is.
    stringstream playerID_msg;
    playerID_msg << "-1 " << (simulRules.currentFrame + simulRules.windowSize) << " 2 " << (sockets.sockets.size() - 1) << "#";
    sockets.sockets.back().write(playerID_msg.str());
    cerr << "player is expected to assume his role at frame " << (simulRules.currentFrame + simulRules.windowSize) << endl;
    
    // send to everyone the ADDHERO msg
    int birth_time = simulRules.currentFrame + simulRules.windowSize;
    
    stringstream createHero_msg;
    createHero_msg << "-1 " << birth_time << " 1#";
    serverMsgs.push_back(createHero_msg.str());
    cerr << "Hero is scheduled for birth at frame " << birth_time << endl;
    
    stringstream clientState_msg;
    clientState_msg << "-2 CLIENT_STATE " << client_state << "#";
    sockets.sockets.back().write(clientState_msg.str());
    
  }
}


