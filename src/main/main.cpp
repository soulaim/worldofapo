

#include <SDL/SDL.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "../localplayer.h"
#include "../logger.h"

using namespace std;

int main()
{
	cerr << "starting logger" << endl;
	Logger log;
	log.open("gamelog.log");
	
	cerr << "creating game object" << endl;
	Localplayer master;
	master.init();

	bool menu = true;
	while(true)
	{
		if(!menu)
		{
			if(master.client_tick())
			{
				menu = true;
			}
		}
		else
		{
			master.menu_tick();
			menu = false;

			cerr << "Menu ended, game starting" << endl;
		}

		master.draw(); // draws if possible
	}
	
	cerr << "apparently exiting now" << endl;
	return 0;
}

