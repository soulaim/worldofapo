
#include "game.h"
#include "image.h"
#include "menubutton.h"

using namespace std;

void Game::menuQuestions()
{
  cerr << "Entering menu" << endl;
  
  
  // load images & create textures
  vector<MenuButton> buttons;
  
  buttons.push_back(MenuButton("host", "data/menu/host.png"));
  buttons.push_back(MenuButton("connect", "data/menu/connect.png"));
  buttons.push_back(MenuButton("exit", "data/menu/exit.png"));
  
  buttons[0].selected = 1;
  int selected = 0;
  
  while(1)
  {
    view.drawMenu(buttons);
    
    string key = userio.getSingleKey();
    
    if(key == "")
    {
      SDL_Delay(50); // sleep a bit. don't need anything intensive done anyway.
      continue;
    }
    else
    {
      cerr << "  pressed key: \"" << key << "\"" << endl;
      
      if(key == "down")
      {
	buttons[selected].selected = 0;
	if(selected == 0)
	  selected = buttons.size() - 1;
	else
	  selected--;
	buttons[selected].selected = 1;
      }
      
      if(key == "up")
      {
	buttons[selected].selected = 0;
	if(selected == (buttons.size()-1))
	  selected = 0;
	else
	  selected++;
	buttons[selected].selected = 1;
      }
 
 
 
      if(key == "return")
      {
	if(buttons[selected].name == "host")
	{
	  makeLocalGame();
	  state = "host";
	}
	
	if(buttons[selected].name == "connect")
	{
	  // ask for host name and connect.
	  cerr << "connecting to an existing game is currently not supported by the menu. sorry." << endl;
	  
	  /*
	  string ip_addr = "128.214.48.248";
	  joinInternetGame(ip_addr);
	  */
	}
	
	if(buttons[selected].name == "exit")
	{
	  cerr << "user selected EXIT from the menu, awesome." << endl;
	  SDL_Quit();
	  exit(0);
	}
	
	break;
      }
 
    }
  }
  
  cerr << "Exiting menu" << endl;
  
  for(int i=0; i<buttons.size(); i++)
  {
    buttons[i].unloadTexture();
  }
  
  return;
  
  
  /*
   // blocks!
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
  
  */
  
}
