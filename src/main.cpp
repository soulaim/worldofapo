

#include <SDL/SDL.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "game.h"

using namespace std;



int main()
{
  cerr << "creating game object" << endl;
  Game master;
  
  while(true)
  {
    master.start();
  }
  
  cerr << "lolwut?" << endl;
  
  return 0;
}
