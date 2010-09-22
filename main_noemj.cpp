

#include "graphics.h"
#include "world.h"

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
  
  cerr << "loading model.." << flush;

  World world;
  world.addUnit();
  
/*
  // actually unit contains a model, but is also described by other information..
  vector<Model> units;
  units.resize(15);
  
  for(int i=0; i<15; i++)
  {
    units[i].load("model.bones");
    units[i].setAction("walk");
    units[i].parts[0].offset_x = (i % 5) * 4;
    units[i].parts[0].offset_z = (i / 5) * 7;
    cerr << units[i].parts.size() << endl;
  }
  
  cerr << "done" << endl;
*/
  
  while(true)
  {
    // fps control
    usleep(10000);
    
    // show current game state
    view.draw(world.models);
    
    for(int i=0; i<world.units.size(); i++)
    {
      world.models[i].tick();
      world.models[i].parts[world.models[i].root].rotation_x = world.units[i].angle;
      world.models[i].parts[world.models[i].root].offset_x = world.units[i].location.x;
      world.models[i].parts[world.models[i].root].offset_z = world.units[i].location.y;
    }
    
    // wait for user IO
    SDL_Event event;
    while ( SDL_PollEvent(&event) )
    {
        switch (event.type) {
            case SDL_MOUSEMOTION:
//                view.mouseMove(event.motion.xrel, event.motion.yrel);
//              event.motion.xrel, event.motion.yrel, event.motion.x, event.motion.y
                break;

            case SDL_MOUSEBUTTONDOWN:
	    {
//	      int select_id = view.mousePress(event.button.button);
              break;
	    }
	    
	    case SDL_KEYDOWN:
		switch( event.key.keysym.sym )
		{
		        case SDLK_RETURN:
			{
			}
			  break;
			
			case SDLK_SPACE:
			  view.setCamera(world.units[0].location);
			  break;
			  
			case SDLK_UP:
			{
			  world.units[0].move(1.);
			}
			break;
			  
			case SDLK_DOWN:
			  break;
			  
			case SDLK_LEFT:
			{
			  world.units[0].angle -= 10;
			}
			  break;
			  
			case SDLK_RIGHT:
			{
			  world.units[0].angle += 10;
			}
			  break;
			
			case SDLK_ESCAPE:
			{
			  // shut down
			  SDL_Quit();
			  exit(0);
			}
		}
	      break;
	      
            case SDL_QUIT:
                exit(0);
        }
    }

  }
  
  return 0;
}
