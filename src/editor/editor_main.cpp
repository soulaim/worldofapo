#include <iostream>

#include "editor.h"
#include "../logger.h"

using namespace std;

int main(int argc, char* argv[])
{
	cerr << "LOL: " << argc << " " << argv << endl;
	
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
