#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>

#include "../localplayer.h"
#include "graphics/menu.h"
#include "graphics/graphics.h"
#include "../userio.h"
#include "graphics/hud.h"
#include "../logger.h"
#include "graphics/window.h"

using namespace std;

string select_hero(Menu& menu, map<string, CharacterInfo>& heroes)
{
	cerr << "Obtained heroes list.. should have a menu here" << endl;
	
	vector<MenuButton> menuButtons;
	
	int dummy_val = -1;
	string dummy_str;
	for(auto iter = heroes.begin(); iter != heroes.end(); iter++)
		menuButtons.push_back(MenuButton(iter->second.playerInfo.name, dummy_str, dummy_val));
	
	string selected_character = menu.run_menu(menuButtons, "Character selection");
	string key = "NEW";
	bool character_found = false;
	
	for(auto iter = heroes.begin(); iter != heroes.end(); iter++)
	{
		if(iter->second.playerInfo.name == selected_character)
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

	return key;
}

bool make_menu_choice(Localplayer& master, Menu& menu, HasProperties& menuOptions, const std::string& choice)
{
	master.endMusic();
	master.startMusic(menuOptions.strVals["GAME_MUSIC"]);
	
	map<string, CharacterInfo> heroes;
	if(!master.internetGameGetHeroes(choice, heroes))
	{
		menu.error_string = "^RConnection failed";
		return true;
	}

	string key = select_hero(menu, heroes);

	master.internetGameSelectHero(key);
	master.reload_confs();

	return false;
}

bool main_loop(Localplayer& master, Menu& menu, HasProperties& menuOptions, Window& window)
{
	static bool in_menu = true;

	if(!in_menu)
	{
		if(master.client_tick())
		{
			
			window.disable_grab();
			
			in_menu = true;
			master.startMusic(menuOptions.strVals["MENU_MUSIC"]);
			
			// at least if server disconnects us, this is the only place where reset is called.
			cerr << "did reset get called before this?" << endl;
			master.reset();
		}
	}
	else
	{
		string choice = menu.menu_tick();
		if(choice == "exit")
		{
			return false;
		}
		else if(!choice.empty())
		{
			in_menu = make_menu_choice(master, menu, menuOptions, choice);
		}
	}
	return true;
}

int main(int argc, char* argv[])
{
	argc = argc;
	argv = argv;

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
	menuOptions.load("configs/menu.conf");
	
	// start music in menu
	master.startMusic(menuOptions.strVals["MENU_MUSIC"]);
	
	while(main_loop(master, menu, menuOptions, window))
	{
		master.draw(); // draws if possible
	}
	
	cerr << "Exiting client now." << endl;
	return 0;
}

