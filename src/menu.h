#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

#include "hasproperties.h"

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
	
	std::vector<HasProperties> options;
	std::vector<std::string> options_files;
	
	std::string getInput(std::vector<MenuButton>&, int);
	int changeValue(std::vector<MenuButton>&, int i);
	
	std::string run_menu(std::vector<MenuButton>& buttons, std::string menu_name);
	
	std::string handleMainMenu(std::vector<MenuButton>& buttons, size_t selected);
	std::string handleOptionsMenu(std::vector<MenuButton>& buttons, size_t selected);
	
	Graphics* view;
	UserIO* userio;
};

#endif

