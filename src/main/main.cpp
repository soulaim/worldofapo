#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>

#include "../localplayer.h"
#include "../menu.h"
#include "../graphics.h"
#include "../userio.h"
#include "../hud.h"
#include "../logger.h"
#include "../window.h"

using namespace std;

int main(int argc, char* argv[])
{
	cerr << "int:" << sizeof(int) << endl;
	cerr << "long:" << sizeof(long) << endl;
	cerr << "long long:" << sizeof(long long) << endl;
	cerr << "LOL: " << argc << " " << argv << endl;
	
	cerr << "starting logger" << endl;
	Logger log;
	log.open("gamelog.log");
	
	cerr << "creating game object" << endl;
	UserIO userio;
	Window window;
	Hud hud;
	Graphics view(window, hud);
	Localplayer master(&view, &userio, &hud, &window);
	master.init();
	Menu menu(&view, &userio);
	
	HasProperties menuOptions;
	menuOptions.load("menu.conf");
	
	// start music in menu
	master.startMusic(menuOptions.strVals["MENU_MUSIC"]);
	
	bool in_menu = true;
	while(true)
	{
		if(!in_menu)
		{
			if(master.client_tick())
			{
				in_menu = true;
				master.startMusic(menuOptions.strVals["MENU_MUSIC"]);
			}
		}
		else
		{
			string choice = menu.menu_tick();
			if(choice == "exit")
			{
				break;
			}
			else if(!choice.empty())
			{
				master.endMusic();
				master.startMusic(menuOptions.strVals["GAME_MUSIC"]);
				
				map<string, string> heroes;
				if(master.internetGameGetHeroes(choice, heroes))
				{
					in_menu = false;
					cerr << "Obtained heroes list.. should have a menu here" << endl;
					
					// TODO: Move this stuff to a separate function. It's getting ugly.
					vector<MenuButton> menuButtons;
					
					int dummy_val = -1;
					string dummy_str;
					for(auto iter = heroes.begin(); iter != heroes.end(); iter++)
						menuButtons.push_back(MenuButton(iter->second, dummy_str, dummy_val));
					
					string selected_character = menu.run_menu(menuButtons, "Character selection");
					string key = "NEW";
					bool character_found = false;
					
					for(auto iter = heroes.begin(); iter != heroes.end(); iter++)
					{
						if(iter->second == selected_character)
						{
							character_found = true;
							key = iter->first;
							break;
						}
					}
					
					if(character_found)
					{
						cerr << "Selected hero OK" << endl;
					}
					else
					{
						cerr << "Selected hero NOT OK" << endl;
					}
					
					// without an actual menu, just select the first option
					cerr << "Selecting hero: " << selected_character << " with key " << key << endl;
					master.internetGameSelectHero(key);
					master.reload_confs();
				}
				else
				{
					menu.error_string = "^RConnection failed";
				}
			}
		}
		
		master.draw(); // draws if possible
	}
	
	cerr << "Exiting client now." << endl;
	return 0;
}

