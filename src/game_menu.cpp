
#include "game.h"
#include "image.h"
#include "menubutton.h"

using namespace std;

void Game::menuQuestions()
{
	cerr << "Entering menu" << endl;
	
	// load images & create textures
	vector<MenuButton> buttons;
	
	buttons.push_back(MenuButton("exit", "data/menu/exit.png"));
	buttons.push_back(MenuButton("connect", "data/menu/connect.png"));
	buttons.push_back(MenuButton("host", "data/menu/host.png"));
	
	int selected = 2;
	bool dont_exit = true;
	buttons[selected].selected = 1;
	
	while(dont_exit)
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
					dont_exit = false;
				}
				
				if(buttons[selected].name == "connect")
				{
					// ask for host name and connect.
					cerr << "Type the name of the host machine: " << endl;
					string hostName = "";
					
					while(true)
					{
						string key_hostname = userio.getSingleKey();
						
						if(key_hostname == "")
						{
							SDL_Delay(50); // sleep a bit. don't need anything intensive done anyway.
							continue;
						}
						else
						{
							if(key_hostname.size() == 1)
								hostName.append(key_hostname);

							if(key_hostname == "escape")
							{
								break;
							}

							if(key_hostname == "backspace")
							{
								if(hostName.size() > 0)
									hostName.resize(hostName.size()-1);
							}

							if(key_hostname == "return")
							{
								joinInternetGame(hostName);
								dont_exit = false;
								break;
							}
							
							cerr << "Current input: \"" << hostName << "\"" << endl;
						}
					}
				}
				
				if(buttons[selected].name == "exit")
				{
					cerr << "user selected EXIT from the menu, awesome." << endl;
					SDL_Quit();
					exit(0);
				}
				
			}
			
		}
	}
	
	cerr << "Exiting menu" << endl;
	
	for(int i=0; i<buttons.size(); i++)
	{
		buttons[i].unloadTexture();
	}
	
	return;
}
