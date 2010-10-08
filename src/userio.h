#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL/SDL.h>

#include <vector>
#include <string>
#include <cstdlib>


struct Coord
{
	Coord(int a, int b): x(a), y(b) {}
	Coord():x(0), y(0) {}
	int x;
	int y;
};

class UserIO
{
	Uint8* keystate;
	int numKeys;
	
	Coord mouseMove;
	Coord mouse;
	int mouseButtons;
	int wheel_has_been_scrolled_up;
	int wheel_has_been_scrolled_down;
	
	std::vector<int> keyStates;
	std::vector<std::string> keyNames;
	std::string emptyString;
public:
	void init();
	
	int getGameInput();
	
	void getMouseChange(int& x, int& y);
	int getMousePress();
	int getMouseWheelScrolled();
	int ismousepressed();
	
	Coord getMousePoint();
	
	int checkEvents();
	std::string& getSingleKey();
	
	UserIO();
};


#endif
