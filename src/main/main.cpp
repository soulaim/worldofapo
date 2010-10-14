

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

	while(true)
	{
		master.client_tick();
		master.draw(); // draws if possible
		
		if(master.in_menu())
		{
			master.menu_tick();
		}
	}
	
	cerr << "apparently exiting now" << endl;
	return 0;
}

