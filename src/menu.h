#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>


class MenuButton;
class Graphics;
class UserIO;

class Menu
{
public:
	std::string menu_tick(); // TODO: separate from localplayer?
	Menu(Graphics*, UserIO*);
	
	std::string error_string;
private:
	std::string connectMenu(std::vector<MenuButton>&);
	Graphics* view;
	UserIO* userio;
};

#endif

