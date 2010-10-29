#include <iostream>

#include "editor.h"

using namespace std;

int main()
{
	Editor master;
	
	while(master.start())
	{
	}
	
	cerr << "Editor stopping." << endl;
	
	return 0;
}
