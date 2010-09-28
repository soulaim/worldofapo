
#include "game.h"

using namespace std;

void Game::menuQuestions()
{
  char val = userio.getSingleChar();
  char str[2];
  str[0] = val;
  str[1] = 0;
  
  // word ready, process the command :D
  if(val == '#')
  {
    stringstream ss(menuWord);
    string word1;
    ss >> word1;
    if(word1 == "connect")
    {
      string ip_addr;
      ss >> ip_addr;
      joinInternetGame(ip_addr);
    }
    
    if(menuWord == "start local")
    {
      cerr << "Hosting a new game.." << endl;
      makeLocalGame();
      state = "host";
    }
    
    // serves as a START button. Could also be used for UNPAUSE functionality?
    if( (menuWord == "go") && (state_descriptor == 0) )
    {
      state_descriptor = 1;
    }
    
    menuWord = "";
    return;
  }
  
  if(val != 0)
  {
    menuWord.append(str);
    cerr << "current menuword: " << menuWord << endl;
  }
  
}
