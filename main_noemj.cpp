

#include "graphics.h"
#include "world.h"
#include "userio.h"

#include <SDL/SDL.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;



int main()
{
  Graphics view;
  cerr << "loading objects.." << flush;
  view.loadObjects("parts.dat");
  view.megaFuck(); // first attempt at animation datastructures, for try-out purposes only..
  cerr << "done" << endl;
  
  World world;
  cerr << "creating ApoMath" << endl;
  world.init();
  cerr << "loading model" << flush;
  world.addUnit();
  
  cerr << "creating controller" << endl;
  UserIO userio;
  
  while(true)
  {
    // fps control
    usleep(10000);
    
    // show current game state
    view.draw(world.models);
    
    for(int i=0; i<world.units.size(); i++)
    {
      world.models[i].tick();
      world.models[i].parts[world.models[i].root].rotation_x = world.units[i].getAngle();
      world.models[i].parts[world.models[i].root].offset_x = world.units[i].position.x.getFloat();
      world.models[i].parts[world.models[i].root].offset_z = world.units[i].position.y.getFloat();
    }
    
    userio.checkEvents();
    int keyState = userio.getKeyChange();
    world.units[0].keyState ^= keyState;
    
    int mx, my;
    userio.getMouseChange(mx, my);
    world.units[0].angle += mx;
    world.models[0].parts[world.models[0].root].rotation_x = world.units[0].getAngle();
    
    world.units[0].tick();
  }
  
  return 0;
}
