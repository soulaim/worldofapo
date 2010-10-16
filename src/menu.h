#ifndef MENU_H
#define MENU_H

#include <string>

#include "graphics.h"
#include "userio.h"

class Menu
{
public:
	std::string menu_tick(); // TODO: separate from localplayer?
	Menu(Graphics*, UserIO*);
private:
	std::string connectMenu();
	Graphics* view;
	UserIO* userio;
};
#endif

