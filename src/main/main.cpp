#include <iostream>
#include <vector>
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

int main()
{
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

	bool in_menu = true;
	while(true)
	{
		if(!in_menu)
		{
			if(master.client_tick())
			{
				in_menu = true;
			}
		}
		else
		{
			string choice = menu.menu_tick();
			if(choice == "exit")
			{
				break;
			}
			else if(!choice.empty() && master.joinInternetGame(choice))
			{
				in_menu = false;
				cerr << "Menu ended, game starting" << endl;
			}
		}

		master.draw(); // draws if possible
	}
	
	cerr << "Exiting client now." << endl;
	return 0;
}

