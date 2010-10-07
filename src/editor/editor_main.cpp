#include <SDL/SDL.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "editor.h"
#include "../logger.h"

using namespace std;

int main()
{
	cerr << "starting logger" << endl;
	Logger log;
	log.open("editor.log");

	cerr << "creating editor object" << endl;
	Editor master;

	while(true)
	{
		master.start();
	}

	cerr << "lolwut?" << endl;

	return 0;
}

