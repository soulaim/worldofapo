#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

#include "hasproperties.h"
#include "random.h"
#include "menubutton.h"

// class MenuButton;
// class MenuParticle;
class GameView;
class UserIO;



class Menu
{
public:
	std::string menu_tick();
	std::string run_menu(std::vector<MenuButton>& buttons, std::string menu_name);
	
	Menu(GameView*, UserIO*);
	
	std::string error_string;
private:
	
	std::vector<HasProperties> options;
	std::vector<std::string> options_files;
	
	// for a background effect on menu screen
	std::vector<MenuParticle> menu_particles;
	
	void tick_menu_effects();
	
	std::string getInput(std::vector<MenuButton>&, int);
	int changeValue(std::vector<MenuButton>&, int i);
	
	std::string handleCharacterSelection(std::vector<MenuButton>& buttons, size_t selected);
	std::string handleMainMenu(std::vector<MenuButton>& buttons, size_t selected);
	std::string handleOptionsMenu(std::vector<MenuButton>& buttons, size_t selected);
	
	GameView* view;
	UserIO* userio;
};

#endif

