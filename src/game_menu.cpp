#include "localplayer.h"
#include "game.h"
#include "image.h"
#include "menubutton.h"

using namespace std;

void Localplayer::menu_tick()
{
	if(game.myID >= 0)
		reset();
	
	// load images & create textures
	vector<MenuButton> buttons;
	
	buttons.push_back(MenuButton("exit", "data/menu/exit.png"));
	buttons.push_back(MenuButton("connect", "data/menu/connect.png"));
	buttons.push_back(MenuButton("host", "data/menu/host.png"));
	
	size_t selected = 2;
	int dont_exit = 1;
	buttons[selected].selected = 1;
	
	while(dont_exit)
	{
		view.drawMenu(buttons);
		
		string key = game.userio.getSingleKey();
		
		if(key == "")
		{
			SDL_Delay(50); // sleep a bit. don't need anything intensive done anyway.
			continue;
		}
		else if(key == "down")
		{
			buttons[selected].selected = 0;
			if(selected == 0)
				selected = buttons.size() - 1;
			else
				selected--;
			buttons[selected].selected = 1;
		}
		else if(key == "up")
		{
			buttons[selected].selected = 0;
			if(selected == (buttons.size()-1))
				selected = 0;
			else
				selected++;
			buttons[selected].selected = 1;
		}
		else if(key == "return")
		{
			if(buttons[selected].name == "connect")
			{
				// ask for host name and connect.
				dont_exit = connectMenu();
			}
			else if(buttons[selected].name == "exit")
			{
				SDL_Quit();
				exit(0);
			}
			else
			{
				cerr << "STUPID USER!!" << endl;
			}
		}
	}
	
	for(size_t i = 0; i < buttons.size(); ++i)
	{
		buttons[i].unloadTexture();
	}
	
	return;
}

int Localplayer::connectMenu()
{
	cerr << "Type the name of the host machine: " << endl;
	string hostName = "";
	
	while(true)
	{
		string key_hostname = game.userio.getSingleKey();
		
		if(key_hostname == "")
		{
			SDL_Delay(50); // sleep a bit. don't need anything intensive done anyway.
			continue;
		}
		else if(key_hostname.size() == 1)
		{
			hostName.append(key_hostname);
		}
		else if(key_hostname == "escape")
		{
			cerr << "Out from connecting" << endl;
			return 1;
		}
		else if(key_hostname == "backspace")
		{
			if(hostName.size() > 0)
				hostName.resize(hostName.size()-1);
		}
		else if(key_hostname == "return")
		{
			joinInternetGame(hostName);
			return 0;
		}
		cerr << "Current input: \"" << hostName << "\"" << endl;
	}
}

