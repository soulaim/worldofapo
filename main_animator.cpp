

#include "graphics.h"
#include <SDL/SDL.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;



int main()
{
  Graphics view;
  cerr << "loading objects.." << flush;
  view.loadObjects("parts.dat");
  view.megaFuck(); // first attempt at animation datastructures, for try-out purposes only..
  cerr << "done" << endl;
  
  cerr << "loading model.." << flush;
  
  // actually unit contains a model, but is also described by other information..
  vector<Model> units;
  units.resize(1);  
  cerr << "done" << endl;

  while(true)
  {
    cerr << "loooooop :DD" << endl;
    
    // fps control
    usleep(10000);
    
    // show current game state
    string selected_parts;
    view.draw(units);
    
    for(int i=0; i<units.size(); i++)
      units[i].tick();
    
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
			  break;
			
			case SDLK_SPACE:
			  break;
			  
			case SDLK_UP:
			  break;
			  
			case SDLK_DOWN:
			{
			  cerr << "Waiting for orders my lord!" << endl;
			  
			  string line;
			  getline(cin, line);
			  stringstream ss(line);
			  
			  string cmd;
			  ss >> cmd;
			  
			  if(cmd == "MODEL")
			  {
			    ss >> cmd;
			    units[0].load(cmd);
			    cerr << units[0].parts.size() << endl;
			  }
			  else if(cmd == "SET")
			  {
			    ss >> cmd;
			    units[0].setAction(cmd);
			    cerr << "Set current animation!" << endl;
			  }
			  else if(cmd == "SELECT")
			  {
			    ss >> selected_parts;
			  }
			  else if(cmd == "ANIMATE")
			  {
			    char dimension; // which dimension..
			    int start, end; // times
			    float start_val, end_val;
			    ss >> dimension >> start >> end >> start_val >> end_val;
			    cerr << dimension << " " << start << " " << end << " " << start_val << " " << end_val << endl;
			    
			    for(int i=0; units[0].parts.size(); i++)
			    {
			      if((units[0].parts[i].wireframe == selected_parts) ||
				 (units[0].parts[i].name == selected_parts)
				)
			      {
				for(int k=start; k<end; k++)
				{
				  cerr << k << endl;
				  ObjectPart& object = view.objects[units[0].parts[i].wireframe];
				  
				  float x = 0., y = 0., z = 0.;
				  if(dimension == 'x')
				    x = start_val + (end_val - start_val) * (k - start) / (float)(end - start);
				  else if(dimension == 'y')
				    y = start_val + (end_val - start_val) * (k - start) / (float)(end - start);
				  else if(dimension == 'z')
				    z = start_val + (end_val - start_val) * (k - start) / (float)(end - start);
				  
				  object.animations[units[0].animation_name].setAnimationState(k, x, y, z);
				}
			      }
			    }
			    
			  }
			  
			} 
			break;
			  
			case SDLK_LEFT:
			  for(int i=0; i<units.size(); i++)
			    units[i].parts[0].rotation_x += 5; // turn
			  break;
			  
			case SDLK_RIGHT:
			  for(int i=0; i<units.size(); i++)
			    units[i].parts[0].rotation_x -= 5; // turn
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
