#include <iostream>

#include "editor.h"
#include "../logger.h"

using namespace std;

int main()
{
	cerr << "Starting logger" << endl;
	Logger log;
	log.open("editor.log");

	Editor master;
	
	while(master.tick())
	{
	}
	
	cerr << "Editor stopping." << endl;
	
	return 0;
}
